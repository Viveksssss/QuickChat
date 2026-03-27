#include <boost/mpl/assert.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include "../global/ConfigManager.h"
#include "../global/const.h"
#include "DistributedLock.h"
#include "RedisManager.h"

RedisManager::~RedisManager() {
    if (_isConnected)
        Close();
}

RedisManager::RedisManager() {
    _host = ConfigManager::GetInstance()["Redis"]["host"];
    _port = std::stoi(ConfigManager::GetInstance()["Redis"]["port"]);
    std::string password = ConfigManager::GetInstance()["Redis"]["password"];
    _pool = std::make_unique<RedisPool>(std::thread::hardware_concurrency(),
                                        _host, _port, password);
    _isConnected = true;
}

bool RedisManager::Get(const std::string &key, std::string &value) {
    auto *reply = execute("GET %s", key.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR || reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        return false;
    }
    value = std::string(reply->str, reply->len);
    freeReplyObject(reply);
    return true;
}

bool RedisManager::Set(const std::string &key, const std::string &value) {
    auto *reply = execute("SET %s %s", key.c_str(), value.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::Auth(const std::string &password) {
    auto *reply = execute("AUTH %s", password.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::LPush(const std::string &key, const std::string &value) {
    auto *reply = execute("LPUSH %s %s", key.c_str(), value.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::LPop(const std::string &key, std::string &value) {
    auto *reply = execute("LPOP %s", key.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    value = std::string(reply->str, reply->len);
    freeReplyObject(reply);
    return true;
}

bool RedisManager::RPush(const std::string &key, const std::string &value) {
    auto *reply = execute("RPUSH %s %s", key.c_str(), value.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::RPop(const std::string &key, std::string &value) {
    auto *reply = execute("RPOP %s", key.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    value = std::string(reply->str, reply->len);
    freeReplyObject(reply);
    return true;
}

bool RedisManager::HSet(const std::string &key, const std::string &hkey,
                        const std::string &value) {
    auto *reply =
        execute("HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::HSet(const char *key, const char *hkey, const char *hvalue,
                        size_t hvaluelen) {
    auto *reply = execute("HSET %s %s %s", key, hkey, hvalue, hvaluelen);
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::HDel(const std::string &key, const std::string &field) {
    auto *reply = execute("HDEL %s %s", key.c_str(), field.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

std::string RedisManager::HGet(const std::string &key,
                               const std::string &hkey) {
    auto *reply = execute("HGET %s %s", key.c_str(), hkey.c_str());
    if (reply == NULL) {
        return "";
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return "";
    }
    std::string value = std::string(reply->str, reply->len);
    freeReplyObject(reply);
    return value;
}

bool RedisManager::Del(const std::string &key) {
    auto *reply = execute("DEL %s", key.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::Decr(const std::string &key, int amount) {
    redisReply *reply = nullptr;
    if (amount == 1) {
        reply = execute("DECR %s", key.c_str());
    } else {
        return this->Set(key, std::to_string((std::stoi(key) - amount)));
    }
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}
bool RedisManager::Incr(const std::string &key, int amount) {
    redisReply *reply = nullptr;
    if (amount == 1) {
        reply = execute("INCR %s", key.c_str());
    } else {
        return this->Set(key, std::to_string((std::stoi(key) + amount)));
    }
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisManager::ExistsKey(const std::string &key) {
    auto *reply = execute("EXISTS %s", key.c_str());
    if (reply == NULL) {
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return false;
    }
    bool exists = (reply->integer == 1);
    freeReplyObject(reply);
    return exists;
}

void RedisManager::Close() {
    if (_pool) {
        _pool->Close();
        _isConnected = false;
        _pool.reset();
    }
}

std::string RedisManager::AcquireLock(const std::string &key, int timeout,
                                      int acquireTimeout) {
    auto connection = _pool->GetConnection();
    if (connection == nullptr) {
        return "";
    }
    Defer defer([&connection, this] { _pool->ReturnConnection(connection); });

    return DistributedLock::GetInstance().AcquireLock(connection, key, timeout,
                                                      acquireTimeout);
}

bool RedisManager::ReleaseLock(const std::string &key,
                               const std::string &identifier) {
    if (identifier.empty()) {
        return false;
    }
    auto connection = _pool->GetConnection();
    if (connection == nullptr) {
        return false;
    }
    Defer defer([&connection, this] { _pool->ReturnConnection(connection); });
    return DistributedLock::GetInstance().ReleaseLock(connection, key,
                                                      identifier);
}

RedisPool::RedisPool(std::size_t size, const std::string &host, int port,
                     const std::string &password)
    : _size(size)
    , _host(host)
    , _port(port)
    , _password(password)
    , _stop(false)
    , _failed_count(0) {
    bool success = true;
    for (std::size_t i = 0; i < _size; ++i) {
        auto *context = CreateConnection();
        if (context == nullptr) {
            success = false;
            _failed_count++;
        }
        if (context) {
            _connections.push(context);
        }
    }

    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    long long timestamp =
        std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
    _last_operate_time = timestamp;

    _check_thread = std::thread([this] {
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

    if (!success) {
        SPDLOG_ERROR("Redis Connect Failed");
    } else {
        SPDLOG_INFO("Redis Connection Pool Initialized");
    }
}

void RedisPool::checkConnection() {
    std::lock_guard<std::mutex> lock(_mutex);
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();

    long long timestamp =
        std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

    if (timestamp - _last_operate_time >= 60) {
        _last_operate_time = timestamp;
        if (_connections.empty() && !_failed_count) {
            return;
        }
        int target = _connections.size();
        for (int i = 0; i < target; ++i) {
            bool health = true;
            auto *conn = _connections.front();
            _connections.pop();
            redisReply *reply;
            try {
                reply = (redisReply *)redisCommand(conn, "PING");
                if (!reply) {
                    SPDLOG_WARN("Redis Exception: disconnected ");
                    _failed_count++;
                    health = false;
                }
                freeReplyObject(reply);
            } catch (const std::exception &e) {
                SPDLOG_ERROR("Redis exception: {}", e.what());
                if (reply) {
                    freeReplyObject(reply);
                }
                _failed_count++;
                health = false;
            }
            if (health) {
                _connections.push(conn);
                _cv.notify_one();
            } else {
                redisFree(conn);
            }
        }

        while (_failed_count > 0) {
            auto b_ok = Reconnect();
            if (b_ok) {
                _failed_count--;
            } else {
                SPDLOG_WARN("Retry Redis connection failed");
                break;
            }
        }
    }
}

bool RedisPool::Reconnect() {
    SPDLOG_INFO("RETRY");
    try {
        auto *context = CreateConnection();
        if (context == nullptr) {
            return false;
        }

        _connections.push(context);
        _cv.notify_one();
        SPDLOG_INFO("Reback new Redis Connection");

        return true;
    } catch (const std::exception &e) {
        SPDLOG_WARN("Reconnect to Redis failed:{}", e.what());
        return false;
    }
}

RedisPool::~RedisPool() { Close(); }

redisContext *RedisPool::CreateConnection() {
    redisContext *context = redisConnect(_host.c_str(), _port);
    if (context == NULL || context->err) {
        if (context)
            redisFree(context);
        return nullptr;
    }
    if (!_password.empty()) {
        auto reply =
            (redisReply *)redisCommand(context, "AUTH %s", _password.c_str());
        if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
            redisFree(context);
            freeReplyObject(reply);
            return nullptr;
        }
    }
    return context;
}

redisContext *RedisPool::GetConnection() {
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this] { return !_connections.empty() || _stop; });
    if (_stop) {
        return nullptr;
    }
    auto *context = _connections.front();
    _connections.pop();
    lock.unlock();
    if (context->err) {
        redisFree(context);
        context = CreateConnection();
    }
    return context;
}

void RedisPool::ReturnConnection(redisContext *context) {
    if (context == nullptr)
        return;
    if (context->err) {
        redisFree(context);
        context = CreateConnection();
        if (!context) {
            return; // 连接池大小暂时减少
        }
    }
    if (context) {
        std::unique_lock<std::mutex> lock(_mutex);
        _connections.push(context);
        _cv.notify_one();
        /*
        在生产者-消费者模式中，先通知后解锁是为了避免丢失唤醒和竞态条件。

        如果先解锁后通知：

        解锁后，其他线程可能立即获取锁并消费资源

        然后才执行通知，这时等待的线程被唤醒但资源已被抢走

        导致虚假唤醒或永久等待

        在锁内通知能保证：线程被唤醒时，资源肯定还在队列中，状态一致性得到保障。这是条件变量的标准用法。
        */
    }
}

void RedisPool::Close() {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_stop) {
            return;
        }
        _stop = true;
        while (!_connections.empty()) {
            auto *context = _connections.front();
            _connections.pop();
            if (context) {
                redisFree(context);
            }
        }
    }
    _cv.notify_all();
    SPDLOG_INFO("Redis Pool Closed");
}

void RedisManager::InitCount(const std::string &server_name) {
    auto lock_key = LOCK_COUNT;
    auto identifier = RedisManager::GetInstance()->AcquireLock(
        lock_key, LOCK_TIMEOUT, ACQUIRE_LOCK_TIMEOUT);

    Defer defer([identifier] {
        RedisManager::GetInstance()->ReleaseLock(LOCK_COUNT, identifier);
    });

    RedisManager::GetInstance()->HSet(LOGIN_COUNT_PREFIX, server_name, "0");
}

void RedisManager::DelCount(const std::string &server_name) {
    auto lock_key = LOCK_COUNT;
    auto identifier = RedisManager::GetInstance()->AcquireLock(
        lock_key, LOCK_TIMEOUT, ACQUIRE_LOCK_TIMEOUT);

    Defer defer([identifier] {
        RedisManager::GetInstance()->ReleaseLock(LOCK_COUNT, identifier);
    });

    RedisManager::GetInstance()->HDel(LOGIN_COUNT_PREFIX, server_name);
}
