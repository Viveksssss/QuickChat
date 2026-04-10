#ifndef REDISMANAGER_H
#define REDISMANAGER_H

#include "../global/Singleton.h"
#include <atomic>
#include <condition_variable>
#include <hiredis/hiredis.h>
#include <memory>
#include <mutex>
#include <queue>
<<<<<<< HEAD
#include <string>
#include <thread>

class RedisPool {
public:
    RedisPool(std::size_t size = std::thread::hardware_concurrency(),
        std::string const &host = "127.0.0.1", int port = 6379,
        std::string const &password = "");
=======
#include <thread>

class RedisPool {
  public:
    RedisPool(std::size_t size = std::thread::hardware_concurrency(),
              const std::string &host = "127.0.0.1", int port = 6379,
              const std::string &password = "");
>>>>>>> origin/main
    ~RedisPool();
    redisContext *CreateConnection();
    redisContext *GetConnection();
    void ReturnConnection(redisContext *context);
    void Close();
    void checkConnection();
    bool Reconnect();

<<<<<<< HEAD
private:
=======
  private:
>>>>>>> origin/main
    int64_t _last_operate_time;
    int _failed_count;
    std::thread _check_thread;
    std::string _host;
    std::size_t _port;
    std::size_t _size;
    std::string _password;
    std::atomic<bool> _stop;
    std::queue<redisContext *> _connections;
    std::mutex _mutex;
    std::condition_variable _cv;
};

class RedisManager : public Singleton<RedisManager> {
    friend class Singleton<RedisManager>;

<<<<<<< HEAD
public:
    ~RedisManager();
    bool Get(std::string const &key, std::string &value);
    bool Set(std::string const &key, std::string const &value);
    bool Auth(std::string const &password);
    bool LPush(std::string const &key, std::string const &value);
    bool LPop(std::string const &key, std::string &value);
    bool RPush(std::string const &key, std::string const &value);
    bool RPop(std::string const &key, std::string &value);
    bool HSet(std::string const &key, std::string const &hkey,
        std::string const &value);
    bool HSet(char const *key, char const *hkey, char const *hvalue,
        size_t hvaluelen);
    bool HDel(std::string const &key, std::string const &field);
    std::string HGet(std::string const &key, std::string const &hkey);
    bool Del(std::string const &key);
    bool Decr(std::string const &key, int amount = 1);
    bool Incr(std::string const &key, int amount = 1);
    bool ExistsKey(std::string const &key);
    void Close();

    inline bool isConnected() const {
        return _isConnected;
    }

    std::string AcquireLock(
        std::string const &key, int timeout = 5, int acquireTimeout = 5);
    bool ReleaseLock(std::string const &key, std::string const &identifier);
    void InitCount(std::string const &server_name);
    void DelCount(std::string const &server_name);

private:
    RedisManager();

    template <typename... Args>
    redisReply *execute(char const *format, Args const &...args) {
        auto *context = _pool->GetConnection();
        redisReply *reply
            = (redisReply *)redisCommand(context, format, args...);
=======
  public:
    ~RedisManager();
    bool Get(const std::string &key, std::string &value);
    bool Set(const std::string &key, const std::string &value);
    bool Auth(const std::string &password);
    bool LPush(const std::string &key, const std::string &value);
    bool LPop(const std::string &key, std::string &value);
    bool RPush(const std::string &key, const std::string &value);
    bool RPop(const std::string &key, std::string &value);
    bool HSet(const std::string &key, const std::string &hkey,
              const std::string &value);
    bool HSet(const char *key, const char *hkey, const char *hvalue,
              size_t hvaluelen);
    bool HDel(const std::string &key, const std::string &field);
    std::string HGet(const std::string &key, const std::string &hkey);
    bool Del(const std::string &key);
    bool Decr(const std::string &key, int amount = 1);
    bool Incr(const std::string &key, int amount = 1);
    bool ExistsKey(const std::string &key);
    void Close();
    inline bool isConnected() const { return _isConnected; }
    std::string AcquireLock(const std::string &key, int timeout = 5,
                            int acquireTimeout = 5);
    bool ReleaseLock(const std::string &key, const std::string &identifier);
    void InitCount(const std::string &server_name);
    void DelCount(const std::string &server_name);

  private:
    RedisManager();
    template <typename... Args>
    redisReply *execute(const char *format, const Args &...args) {
        auto *context = _pool->GetConnection();
        redisReply *reply =
            (redisReply *)redisCommand(context, format, args...);
>>>>>>> origin/main
        _pool->ReturnConnection(context);
        return reply;
    }

<<<<<<< HEAD
private:
=======
  private:
>>>>>>> origin/main
    std::string _host;
    std::size_t _port;
    bool _isConnected = false;
    std::unique_ptr<RedisPool> _pool;
};

#endif
