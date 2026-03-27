#include "MysqlDao.h"
#include "../global/ConfigManager.h"
#include "../global/const.h"
#include <spdlog/spdlog.h>

MysqlPool::MysqlPool(const std::string &url, const std::string &user,
                     const std::string &password, const std::string &schedma,
                     const std::string &port, int poolSize)
    : _url(url)
    , _user(user)
    , _password(password)
    , _schedma(schedma)
    , _port(port)
    , _poolSize(poolSize)
    , _stop(false)
    , _last_operate_time()
    , _failed_count(0) {
    for (std::size_t i = 0; i < _poolSize; ++i) {
        try {
            auto conn = std::make_unique<mysqlpp::Connection>();
            if (conn->connect(_schedma.c_str(), _url.c_str(), _user.c_str(),
                              _password.c_str(), std::stoi(_port))) {
                _connections.push(std::move(conn));
            } else {
                SPDLOG_ERROR("Failed To Create Database Connection: {}",
                             conn->error());
            }
        } catch (const mysqlpp::Exception &e) {
            SPDLOG_ERROR("Failed to connect to mysql:{}", e.what());
        }
    }
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    long long timestamp =
        std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
    _last_operate_time = timestamp;

    if (_connections.size() < _poolSize) {
        SPDLOG_WARN("Connection Pool Initialized With Only {}/{} Connections",
                    _connections.size(), _poolSize);
    } else {
        SPDLOG_INFO("Mysql Connection Pool Initialized");
    }

    _check_thread = std::thread([this]() {
        int count = 0;
        while (!_stop) {
            if (count >= 10) {
                count = 0;
                checkConnection();
            }
            count++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    _check_thread.detach();
}

long long MysqlPool::GetLastOperateTime() { return _last_operate_time; }

void MysqlPool::checkConnection() {
    std::lock_guard<std::mutex> lock(_mutex);
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();

    long long timestamp =
        std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

    if (timestamp - _last_operate_time >= 10) {
        _last_operate_time = timestamp;
        if (_connections.empty() && !_failed_count) {
            return;
        }
        int target = _connections.size();
        for (int i = 0; i < target; ++i) {
            bool health = true;
            auto conn = std::move(_connections.front());
            _connections.pop();
            try {
                mysqlpp::Query query = conn->query();
                query << "SELECT 1";
                query.parse();
                auto res = query.store();
            } catch (const std::exception &e) {
                SPDLOG_ERROR("MySQL++ exception: {}", e.what());
                _failed_count++;
                health = false;
            }
            if (health) {
                _connections.push(std::move(conn));
                _cv.notify_one();
            }
        }

        while (_failed_count > 0) {
            auto b_ok = Reconnect();
            if (b_ok) {
                _failed_count--;
            } else {
                break;
            }
        }
    }
}

bool MysqlPool::Reconnect() {
    SPDLOG_INFO("RETRY");
    try {
        auto conn = std::make_unique<mysqlpp::Connection>();
        if (conn->connect(_schedma.c_str(), _url.c_str(), _user.c_str(),
                          _password.c_str(), std::stoi(_port))) {
            _connections.push(std::move(conn));
            return true;
        }
        SPDLOG_WARN("Reconnect to MYSQL failed:{}", conn->error());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_WARN("Reconnect to MYSQL failed:{}", e.what());
        return false;
    }
}
MysqlPool::~MysqlPool() { Close(); }

std::unique_ptr<mysqlpp::Connection> MysqlPool::GetConnection() noexcept {
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this] { return _stop || !_connections.empty(); });
    if (_stop) {
        return nullptr;
    }
    auto conn = std::move(_connections.front());
    _connections.pop();
    return conn;
}

void MysqlPool::ReturnConnection(
    std::unique_ptr<mysqlpp::Connection> conn) noexcept {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_stop) {
        return;
    }
    _connections.push(std::move(conn));
    _cv.notify_one();
}

void MysqlPool::Close() noexcept {
    std::unique_lock<std::mutex> lock(_mutex);
    _stop = true;
    _cv.notify_all();
    while (!_connections.empty()) {
        auto p = std::move(_connections.front());
        _connections.pop();
    }
}

MysqlDao::MysqlDao() {
    auto &cfgMgr = ConfigManager::GetInstance();
    const auto &host = cfgMgr["Mysql"]["host"];
    const auto &port = cfgMgr["Mysql"]["port"];
    const auto &schema = cfgMgr["Mysql"]["schema"];
    const auto &password = cfgMgr["Mysql"]["password"];
    const auto &user = cfgMgr["Mysql"]["user"];
    _pool = std::make_unique<MysqlPool>(host, user, password, schema);
}

MysqlDao::~MysqlDao() { _pool->Close(); }

int MysqlDao::TestUidAndEmail(const std::string &uid,
                              const std::string &email) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        return -1;
    }
    Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();
        query << "select * from user where uid = %0q or email = %1q";
        query.parse();

        mysqlpp::StoreQueryResult res = query.store(uid, email);

        std::size_t count = res.num_rows();
        if (count != 1) {
            return -1;
        }
        return 1;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("MySQL++ exception in TestUidAndEmail: {}", e.what());
        return -1;
    }
}

int MysqlDao::RegisterUser(const std::string &name, const std::string &email,
                           const std::string &password) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        return -1;
    }
    Defer defer([&conn, this] { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Transaction trans(*conn);
        mysqlpp::Query query = conn->query();

        // 先检查是否用户已经存在
        query << "SELECT id FROM user WHERE name = %0q OR email = %1q FOR "
                 "UPDATE";
        auto check_result = query.store(name, email);

        if (check_result && check_result.num_rows() > 0) {
            trans.rollback();
            return -1;
        }

        // 如果不存在就插入，注册成功
        query << "INSERT INTO user (name, email, password) VALUES (%0q, %1q, "
                 "%2q)";
        auto insert_result = query.execute(name, email, password);

        if (insert_result) {
            int new_id = query.insert_id();
            trans.commit();
            return new_id;
        } else {
            trans.rollback();
            return -1;
        }

    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return -1;
    }
}

int MysqlDao::ResetPassword(const std::string &email,
                            const std::string &password) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        return -1;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return -1;
    }

    try {
        // 使用 mysql++ 预处理语句
        mysqlpp::Query query = conn->query();
        query << "UPDATE user SET password = %0q WHERE email = %1q";

        // 执行预处理更新
        mysqlpp::SimpleResult res = query.execute(password, email);

        if (res) {
            int affected_rows = res.rows();
            if (affected_rows > 0) {
                SPDLOG_INFO("Password reset successfully for email: {}, "
                            "affected rows: {}",
                            email, affected_rows);
                return 1; // 成功重置密码
            } else {
                SPDLOG_WARN("No user found with email: {}", email);
                return 0; // 没有找到用户，返回0
            }
        } else {
            SPDLOG_ERROR("Failed to reset password: {}", query.error());
            return -1;
        }

    } catch (const mysqlpp::BadQuery &e) {
        SPDLOG_ERROR("Bad query in ResetPassword: {}", e.what());
        return -1;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception in ResetPassword: {}", e.what());
        return -1;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception in ResetPassword: {}", e.what());
        return -1;
    }
}

bool MysqlDao::CheckPwd(const std::string &user, const std::string &password,
                        UserInfo &userInfo) {
    // chatServer暂时用不到。
    auto conn = _pool->GetConnection();
    if (!conn) {
        return false;
    }

    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });

    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    try {
        mysqlpp::Query query = conn->query();
        query << "Select * from user where (uid = %0q or email = %1q) and "
                 "password = %2q";
        query.parse();
        mysqlpp::StoreQueryResult res = query.store(user, user, password);
        std::size_t count = res.num_rows();
        if (count != 1) {
            return false;
        }
        return true;
    } catch (const mysqlpp::BadQuery &e) {
        SPDLOG_ERROR("Bad query in CheckPwd: {}", e.what());
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception in CheckPwd: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception in CheckPwd: {}", e.what());
        return false;
    }
}
