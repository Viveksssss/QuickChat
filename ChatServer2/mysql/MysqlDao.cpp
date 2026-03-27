#include "MysqlDao.h"
#include "../data/UserInfo.h"
#include "../data/im.pb.h"
#include "../global/ConfigManager.h"
#include "../global/const.h"
#include <chrono>
#include <cstdlib>
#include <exception>
#include <mysql++/connection.h>
#include <mysql++/exceptions.h>
#include <mysql++/query.h>
#include <mysql++/result.h>
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
                _failed_count++;
            }
        } catch (const mysqlpp::Exception &e) {
            SPDLOG_ERROR("Failed to connect to mysql:{}", e.what());
            _failed_count++;
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
            if (count >= 60 * 5) {
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

    if (timestamp - _last_operate_time >= 30) {
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
            _cv.notify_one();
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
    _pool = std::make_unique<MysqlPool>(host, user, password, schema, port);
}

MysqlDao::~MysqlDao() { _pool->Close(); }

bool MysqlDao::test() {
    auto conn = _pool->GetConnection();
    if (!conn) {
        return -1;
    }
    Defer defer([this, &conn] { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();
        query << "SELECT NOW()";

        mysqlpp::StoreQueryResult res = query.store();

        std::size_t count = res.num_rows();
        if (count != 1) {
            return -1;
        }
        return true;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("MySQL++ exception in TestUidAndEmail: {}", e.what());
        return false;
    }
}

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

bool MysqlDao::AddFriendApply(const std::string &fromUid,
                              const std::string &toUid) {
    if (fromUid == toUid) {
        return false; // 不允许自己添加自己
    }
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }

    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();
        query << "INSERT IGNORE INTO friend_apply (from_uid,to_uid) "
                 "VALUES(%0,%1) ";
        query.parse();

        mysqlpp::SimpleResult res =
            query.execute(std::stoi(fromUid), std::stoi(toUid));
        int rowCount = res.rows();
        return rowCount >= 0;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    }

    return true;
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return nullptr;
    }

    try {
        mysqlpp::Query query = conn->query();
        query << "SELECT * FROM user WHERE uid = %0q";

        query.parse();
        mysqlpp::StoreQueryResult res = query.store(uid);

        if (res && res.num_rows() == 1) {
            auto user_info = std::make_shared<UserInfo>();
            user_info->uid = uid;
            user_info->sex = res[0]["sex"];
            user_info->status = res[0]["status"];
            user_info->name = ValueOrEmpty(std::string(res[0]["name"]));
            user_info->email = ValueOrEmpty(std::string(res[0]["email"]));
            user_info->icon = ValueOrEmpty(std::string(res[0]["icon"]));
            user_info->desc = ValueOrEmpty(std::string(res[0]["desc"]));
            // user_info->nick = ValueOrEmpty(std::string(res[0]["nick"]));

            _pool->ReturnConnection(std::move(conn));
            return user_info;
        } else {
            _pool->ReturnConnection(std::move(conn));
            SPDLOG_DEBUG("User not found with uid: {}", uid);
            return nullptr;
        }

    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        if (conn)
            _pool->ReturnConnection(std::move(conn));
        return nullptr;
    }
}

std::vector<std::shared_ptr<UserInfo>>
MysqlDao::GetUser(const std::string &name) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return {};
    }

    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();

        // 使用预处理语句进行模糊查询
        query << "SELECT * FROM user WHERE name LIKE " << mysqlpp::quote
              << ("%" + name + "%");

        mysqlpp::StoreQueryResult res = query.store();
        std::vector<std::shared_ptr<UserInfo>> users;

        if (res) {
            users.reserve(res.num_rows()); // 预分配内存
            for (size_t i = 0; i < res.num_rows(); ++i) {
                auto user_info = std::make_shared<UserInfo>();
                user_info->uid = res[i]["uid"];
                user_info->sex = res[i]["sex"];
                user_info->status = res[i]["status"];
                user_info->name = ValueOrEmpty(std::string(res[i]["name"]));
                user_info->email = ValueOrEmpty(std::string(res[i]["email"]));
                user_info->icon = ValueOrEmpty(std::string(res[i]["icon"]));
                user_info->desc = ValueOrEmpty(std::string(res[i]["desc"]));
                // user_info->nick =
                // ValueOrEmpty(std::string(res[i]["nick"]));

                users.push_back(user_info);
            }
            SPDLOG_DEBUG("Found {} users matching pattern: '{}'", users.size(),
                         name);
        }
        return users;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return {};
    }
}

bool MysqlDao::GetFriendApplyList(
    const std::string &uid, std::vector<std::shared_ptr<UserInfo>> &applyList) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }

    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();
        query << "SELECT u.uid, u.name, u.icon, u.desc, u.sex, fa.time "
              << "FROM user u, friend_apply fa "
              << "WHERE u.uid = fa.from_uid " // 获取申请发送者的信息
              << "AND fa.to_uid = %0q "       // 当前用户是接收方
              << "AND fa.status = 0 "         // status = 0 表示等待处理
              << "ORDER BY fa.time desc";
        query.parse();
        mysqlpp::StoreQueryResult res = query.store(uid);

        if (res && res.num_rows() > 0) {
            applyList.reserve(res.num_rows()); // 预分配内存
            for (size_t i = 0; i < res.num_rows(); ++i) {
                auto user_info = std::make_shared<UserInfo>();
                user_info->uid = res[i]["uid"];
                user_info->sex = res[i]["sex"];
                user_info->name = ValueOrEmpty(std::string(res[i]["name"]));
                user_info->icon = ValueOrEmpty(std::string(res[i]["icon"]));
                user_info->desc = ValueOrEmpty(std::string(res[i]["desc"]));
                user_info->back = ValueOrEmpty(std::string(res[i]["time"]));
                applyList.push_back(user_info);
            }
            return true;
        }
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::CheckApplied(const std::string &fromUid,
                            const std::string &toUid) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();
        query << "SELECT COUNT(*) as cnt from friend_apply where from_uid = "
                 "%0q and to_uid = %1q";
        query.parse();
        mysqlpp::StoreQueryResult res =
            query.store(std::stoi(fromUid), std::stoi(toUid));
        if (res && res.num_rows() == 1) {
            int count = res[0]["cnt"];
            return count > 0;
        }
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::ChangeMessageStatus(const std::string &uid, int status) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();
        query << "UPDATE notifications SET status = %0q WHERE uid = %1q";
        query.parse();
        mysqlpp::SimpleResult res = query.execute(status, std::stoi(uid));
        if (res) {
            int affected_rows = res.rows();
            if (affected_rows > 0) {
                SPDLOG_INFO("Message status changed successfully for uid: {}, "
                            "status: {}",
                            uid, status);
                return true;
            } else {
                SPDLOG_WARN("No message found with uid: {}", uid);
                return false;
            }
        } else {
            SPDLOG_ERROR("Failed to change message status: {}", query.error());
            return false;
        }
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::ChangeApplyStatus(const std::string &fromUid,
                                 const std::string &toUid, int status) {
    if (!status || fromUid == toUid) {
        return false;
    }
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });

    try {
        mysqlpp::Query query = conn->query();
        query << "UPDATE friend_apply SET status = %0q WHERE from_uid = %1q "
                 "AND to_uid = %2q";
        query.parse();
        mysqlpp::SimpleResult res =
            query.execute(status, std::stoi(fromUid), std::stoi(toUid));
        if (res) {
            int affected_rows = res.rows();
            if (affected_rows > 0) {
                SPDLOG_INFO("Apply status changed successfully for from_uid: "
                            "{}, to_uid: {}, status: {}",
                            fromUid, toUid, status);
                return true;
            } else {
                SPDLOG_WARN("No apply found with from_uid: {}, to_uid: {}",
                            fromUid, toUid);
                return false;
            }
        } else {
            SPDLOG_ERROR("Failed to change apply status: {}", query.error());
            return false;
        }

    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::CheckIsFriend(const std::string &fromUid,
                             const std::string &toUid) {
    if (fromUid == toUid) {
        return true;
    }
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        query << "SELECT COUNT(*) as cnt from friends where (self_id = %0q "
                 "and "
                 "friend_id = %1q)"
              << "OR (self_id = %1q and friend_id = %0q)";
        query.parse();
        mysqlpp::StoreQueryResult res =
            query.store(std::stoi(fromUid), std::stoi(toUid));
        if (res && !res.empty()) {
            int count = res[0]["cnt"];
            return count == 2;
        }
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::AddNotification(const std::string &uid, int type,
                               const std::string &message) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        query << "INSERT INTO notifications (uid,type,message) "
                 "VALUES(%0q,%1q,%2q)";
        query.parse();
        mysqlpp::SimpleResult res =
            query.execute(std::stoi(uid), type, message);
        if (res) {
            int affected_rows = res.rows();
            if (affected_rows > 0) {
                SPDLOG_INFO("Notification added successfully for uid: {}, "
                            "type: {}, message: {}",
                            uid, type, message);
                return true;
            } else {
                SPDLOG_WARN("Failed to add notification for uid: {}, type: {}, "
                            "message: {}",
                            uid, type, message);
                return false;
            }
        } else {
            SPDLOG_ERROR("Failed to add notification: {}", query.error());

            return false;
        }
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::GetNotificationList(
    const std::string &uid,
    std::vector<std::shared_ptr<UserInfo>> &notificationList) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        query << "SELECT u.uid,u.name,u.icon,u.sex,n.type,n.message,n.time "
                 "from"
              << " user u, notifications n"
              << " WHERE u.uid = n.uid AND u.uid = %0q"
              << " AND n.status = 0"
              << " ORDER BY n.time DESC";
        query.parse();
        mysqlpp::StoreQueryResult res = query.store(std::stoi(uid));
        int count = res.num_rows();
        if (res && res.num_rows() > 0) {
            notificationList.reserve(res.num_rows()); // 预分配内存
            for (size_t i = 0; i < res.num_rows(); ++i) {
                auto user_info = std::make_shared<UserInfo>();
                user_info->uid = res[i]["uid"];
                user_info->status = res[i]["type"];
                user_info->desc = ValueOrEmpty(std::string(res[i]["message"]));
                user_info->back = ValueOrEmpty(std::string(res[i]["time"]));
                user_info->sex = res[i]["sex"];
                user_info->name = ValueOrEmpty(std::string(res[i]["name"]));
                user_info->icon = ValueOrEmpty(std::string(res[i]["icon"]));
                notificationList.push_back(user_info);
            }
            return true;
        }
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::MakeFriends(const std::string &fromUid,
                           const std::string &toUid) {
    if (fromUid == toUid) {
        return false;
    }

    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Transaction trans(*conn);
        // 添加好友应该是双向的，所以需要插入两条记录
        mysqlpp::Query query1 = conn->query();
        query1 << "INSERT IGNORE INTO friends (self_id,friend_id) "
                  "VALUES(%0q,%1q),"
               << "(%1q,%0q)";
        query1.parse();
        mysqlpp::SimpleResult res1 =
            query1.execute(std::stoi(fromUid), std::stoi(toUid));
        if (res1) {
            int affected_rows1 = res1.rows();
            if (affected_rows1 > 0) {
                trans.commit();
                SPDLOG_INFO("Friends added successfully for from_uid: {}, "
                            "to_uid: {}",
                            fromUid, toUid);
                return true;
            } else {
                trans.rollback();
                SPDLOG_WARN(
                    "Failed to add friends for from_uid: {}, to_uid: {}",
                    fromUid, toUid);
                return false;
            }
        } else {
            trans.rollback();
            SPDLOG_ERROR("Failed to add friends for from_uid: {}, to_uid: {}",
                         fromUid, toUid);
            return false;
        }
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::GetFriendList(
    const std::string &uid,
    std::vector<std::shared_ptr<UserInfo>> &friendList) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        // 使用显式JOIN，更清晰
        query << "SELECT u.uid, u.name, u.icon, u.email, u.sex, u.desc,u.back"
              << " FROM user u"
              << " INNER JOIN friends f ON u.uid = f.friend_id"
              << " WHERE f.self_id = %0q"
              << " ORDER BY f.friend_id DESC";
        query.parse();
        mysqlpp::StoreQueryResult res = query.store(std::stoi(uid));
        int count = res.num_rows();
        if (res && res.num_rows() > 0) {
            friendList.reserve(res.num_rows()); // 预分配内存
            for (size_t i = 0; i < res.num_rows(); ++i) {
                auto user_info = std::make_shared<UserInfo>();
                user_info->uid = res[i]["uid"];
                user_info->sex = res[i]["sex"];
                user_info->name = ValueOrEmpty(std::string(res[i]["name"]));
                user_info->icon = ValueOrEmpty(std::string(res[i]["icon"]));
                user_info->email = ValueOrEmpty(std::string(res[i]["email"]));
                user_info->desc = ValueOrEmpty(std::string(res[i]["desc"]));
                user_info->back = ValueOrEmpty(std::string(res[i]["back"]));
                friendList.push_back(user_info);
            }
            return true;
        }
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::AddMessage(const std::string &uid, int from_uid, int to_uid,
                          const std::string &timestamp, int env,
                          int content_type, const std::string &content_data,
                          const std::string &content_mime_type,
                          const std::string &content_fid, int status) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        query << "INSERT INTO messages "
                 "(uid,from_uid,to_uid,timestamp,env,content_type,content_"
                 "data,"
                 "content_mime_type,content_fid,status) "
                 "VALUES(%0q,%1q,%2q,%3q,%4q,%5q,%6q,%7q,%8q,%9q)";
        query.parse();
        mysqlpp::SimpleResult res =
            query.execute(uid, from_uid, to_uid, timestamp, env, content_type,
                          content_data, content_mime_type, content_fid, status);
        if (res) {
            int affected_rows = res.rows();
            if (affected_rows > 0) {
                SPDLOG_INFO(
                    "Message added successfully for from_uid: {}, to_uid: "
                    "{}, "
                    "timestamp: {}, env: {}, content_type: {}, "
                    "content_data: "
                    "{}, content_mime_type: {}, fid: {}, status: {}",
                    from_uid, to_uid, timestamp, env, content_type,
                    content_data, content_mime_type, content_fid, status);
                return true;
            } else {
                SPDLOG_WARN(
                    "Failed to add message for from_uid: {}, to_uid: {}, "
                    "timestamp: {}, env: {}, content_type: {}, "
                    "content_data: "
                    "{}, content_mime_type: {}, fid: {}, status: {}",
                    from_uid, to_uid, timestamp, env, content_type,
                    content_data, content_mime_type, content_fid, status);
                return false;
            }
        } else {
            SPDLOG_ERROR("Failed to add message: {}", query.error());
            return false;
        }
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::AddConversation(const std::string &uid, int from_uid, int to_uid,
                               const std::string &create_time,
                               const std::string &update_time,
                               const std::string &name, const std::string &icon,
                               int staus, int deleted, int pined,
                               bool processed) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        query << "INSERT INTO conversations "
                 "(uid,from_uid,to_uid,create_time,update_time,name,icon,"
                 "status,deleted,pined,processed) "
                 "VALUES(%0q,%1q,%2q,%3q,%4q,%5q,%6q,%7q,%8q,%9q,%10)";
        query.parse();
        mysqlpp::SimpleResult res =
            query.execute(uid, from_uid, to_uid, create_time, update_time, name,
                          icon, staus, deleted, pined, processed);
        if (res) {
            int affected_rows = res.rows();
            if (affected_rows > 0) {
                SPDLOG_INFO("Conversation added successfully for uid: {}, "
                            "from_uid: "
                            "{}, to_uid: {}, create_time: {}, update_time: {}, "
                            "name: "
                            "{}, icon: {}, status: {}, deleted: {}, pined: {}",
                            uid, from_uid, to_uid, create_time, update_time,
                            name, icon, staus, deleted, pined);
                return true;
            } else {
                SPDLOG_WARN(
                    "Failed to add conversation for uid: {}, from_uid: {}, "
                    "to_uid: {}, create_time: {}, update_time: {}, name: "
                    "{}, "
                    "icon: {}, status: {}, deleted: {}, pined: {}",
                    uid, from_uid, to_uid, create_time, update_time, name, icon,
                    staus, deleted, pined);
                return false;
            }
        } else {
            SPDLOG_ERROR("Failed to add conversation: {}", query.error());
            return false;
        }
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::GetSeessionList(
    const std::string &uid,
    std::vector<std::shared_ptr<SessionInfo>> &sessionList) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        query << "SELECT * FROM conversations"
              << " WHERE (from_uid = %0q AND deleted = 0)";
        query.parse();
        mysqlpp::StoreQueryResult res = query.store(std::stoi(uid));
        int count = res.num_rows();
        if (res && res.num_rows() > 0) {
            sessionList.reserve(res.num_rows()); // 预分配内存
            for (size_t i = 0; i < res.num_rows(); ++i) {
                auto session_info = std::make_shared<SessionInfo>();
                session_info->uid = res[i]["uid"].c_str();
                session_info->from_uid = res[i]["from_uid"];
                session_info->to_uid = res[i]["to_uid"];
                session_info->create_time = res[i]["create_time"].c_str();
                session_info->update_time = res[i]["update_time"].c_str();
                session_info->name = ValueOrEmpty(std::string(res[i]["name"]));
                session_info->icon = ValueOrEmpty(std::string(res[i]["icon"]));
                session_info->status = res[i]["status"];
                session_info->deleted = res[i]["deleted"];
                session_info->pined = res[i]["pined"];
                session_info->processed = res[i]["processed"];
                sessionList.push_back(session_info);
            }
            return true;
        }
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::GetUnreadMessages(
    const std::string &uid,
    std::vector<std::shared_ptr<im::MessageItem>> &unreadMessages) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Transaction trans(*conn);
        mysqlpp::Query query = conn->query();
        mysqlpp::Query query2 = conn->query();

        query << "SELECT * FROM messages"
              << " WHERE to_uid = %0q AND status = 0";
        query2 << "UPDATE messages SET status = 1 WHERE to_uid = %0q AND "
                  "status = 0";
        query.parse();
        query2.parse();
        mysqlpp::StoreQueryResult res = query.store(std::stoi(uid));
        mysqlpp::SimpleResult res2 = query2.execute(std::stoi(uid));
        int count = res.num_rows();
        if (res && res.num_rows() > 0 && res2) {
            unreadMessages.reserve(res.num_rows()); // 预分配内存
            for (size_t i = 0; i < res.num_rows(); ++i) {
                auto message_item = std::make_shared<im::MessageItem>();
                message_item->set_id(res[i]["uid"].c_str());
                message_item->set_from_id(res[i]["from_uid"]);
                message_item->set_to_id(res[i]["to_uid"]);
                message_item->set_timestamp(res[i]["timestamp"].c_str());
                message_item->set_env(res[i]["env"]);
                message_item->mutable_content()->set_type(
                    res[i]["content_type"]);
                message_item->mutable_content()->set_data(
                    res[i]["content_data"].c_str());
                message_item->mutable_content()->set_mime_type(
                    res[i]["content_mime_type"].c_str());
                message_item->mutable_content()->set_fid(
                    res[i]["content_fid"].c_str());
                unreadMessages.push_back(message_item);
            }
            trans.commit();
            return true;
        }
        trans.rollback();
        return false;
    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

bool MysqlDao::ChangeProfile(int uid, int sex, const std::string &name,
                             const std::string &email,
                             const std::string &avatar,
                             const std::string &desc) {
    auto conn = _pool->GetConnection();
    if (!conn) {
        SPDLOG_ERROR("Failed to get connection from pool");
        return false;
    }
    Defer defer([this, &conn]() { _pool->ReturnConnection(std::move(conn)); });
    try {
        mysqlpp::Query query = conn->query();
        query << "update user set "
              << "sex = %0q,"
              << "name = %1q,"
              << "email = %2q,"
              << "icon = %3q,"
              << "desc = %4q,"
              << "where uid = %5";
        query.parse();
        mysqlpp::SimpleResult res =
            query.execute(sex, name, email, avatar, desc, uid);
        if (res) {
            return true;
        }
        return false;

    } catch (const mysqlpp::Exception &e) {
        SPDLOG_ERROR("MySQL++ exception: {}", e.what());
        return false;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return false;
    }
}

std::string MysqlDao::ValueOrEmpty(std::string value) {
    if (value == "null" || value == "NULL" || value == "EMPTY") {
        return "";
    }
    return value;
}
