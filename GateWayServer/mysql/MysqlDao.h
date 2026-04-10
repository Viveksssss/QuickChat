#ifndef MYSQLDAO_H
#define MYSQLDAO_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include <queue>
#include <string>
#include <thread>

/**
 ┌─────────────────┐       ┌──────────────────┐       ┌─────────────────┐
 │   MysqlDao      │       │   MySqlPool      │       │ UserInfo (DTO)  │
 │                 │       │                  │       │                 │
 │ - pool_         │───────│ - connections    │       │ - name          │
 │                 │       │ - pool params    │       │ - email         │
 │ + RegUser()     │       │ + getConnection()│       │ - password      │
 │ + other CRUD    │       │ + returnConn()   │       │                 │
 └─────────────────┘       └──────────────────┘       └─────────────────┘
          │                           │
          │                           │
          ▼                           ▼
 ┌─────────────────┐       ┌──────────────────┐
 │  Business Logic │       │ mysql::Connection│
 │   (Service层)   │       │  (MySQL驱动)      │
 └─────────────────┘       └──────────────────┘
 *
 */

class MysqlPool {
<<<<<<< HEAD
public:
    MysqlPool(std::string const &url, std::string const &user,
        std::string const &password, std::string const &schedma,
        std::string const &port = "3306",
        int poolSize = std::thread::hardware_concurrency());
=======
  public:
    MysqlPool(const std::string &url, const std::string &user,
              const std::string &password, const std::string &schedma,
              const std::string &port = "3306",
              int poolSize = std::thread::hardware_concurrency());
>>>>>>> origin/main

    std::unique_ptr<mysqlpp::Connection> GetConnection() noexcept;
    void ReturnConnection(std::unique_ptr<mysqlpp::Connection> conn) noexcept;
    void Close() noexcept;
    void checkConnection();
    bool Reconnect();
    long long GetLastOperateTime();
    ~MysqlPool();

<<<<<<< HEAD
private:
=======
  private:
>>>>>>> origin/main
    int64_t _last_operate_time;
    int _failed_count;
    std::string _schedma;
    std::string _user;
    std::string _password;
    std::string _url;
    std::string _port;
    std::size_t _poolSize;
    std::queue<std::unique_ptr<mysqlpp::Connection>> _connections;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<bool> _stop;
    std::thread _check_thread;
};

struct UserInfo {
    std::string name;
    std::string pwd;
    int uid;
    std::string email;
};

class MysqlDao {
<<<<<<< HEAD
public:
    MysqlDao();
    ~MysqlDao();
    int TestUidAndEmail(std::string const &uid, std::string const &email);
    int RegisterUser(std::string const &name, std::string const &email,
        std::string const &password);
    int ResetPassword(std::string const &email, std::string const &password);
    bool CheckPwd(std::string const &user, std::string const &password,
        UserInfo &userInfo);

private:
=======
  public:
    MysqlDao();
    ~MysqlDao();
    int TestUidAndEmail(const std::string &uid, const std::string &email);
    int RegisterUser(const std::string &name, const std::string &email,
                     const std::string &password);
    int ResetPassword(const std::string &email, const std::string &password);
    bool CheckPwd(const std::string &user, const std::string &password,
                  UserInfo &userInfo);

  private:
>>>>>>> origin/main
    std::unique_ptr<MysqlPool> _pool;
};

#endif
