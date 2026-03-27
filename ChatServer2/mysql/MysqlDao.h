#ifndef MYSQLDAO_H
#define MYSQLDAO_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#define MYSQLPP_MYSQL_HEADERS_BURIED
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
// #include <mysql/mysql.h> // 移除c的封装改用cpp

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

namespace im {
class MessageItem;
}

struct UserInfo;
struct SessionInfo;
class MysqlPool {
  public:
    MysqlPool(const std::string &url, const std::string &user,
              const std::string &password, const std::string &schedma,
              const std::string &port = "3306",
              int poolSize = std::thread::hardware_concurrency());

    std::unique_ptr<mysqlpp::Connection> GetConnection() noexcept;
    void ReturnConnection(std::unique_ptr<mysqlpp::Connection> conn) noexcept;
    void Close() noexcept;
    void checkConnection();
    bool Reconnect();
    long long GetLastOperateTime();
    ~MysqlPool();

  private:
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

class MysqlDao {
  public:
    MysqlDao();
    ~MysqlDao();
    bool test();
    int TestUidAndEmail(const std::string &uid, const std::string &email);
    int RegisterUser(const std::string &name, const std::string &email,
                     const std::string &password);
    int ResetPassword(const std::string &email, const std::string &password);
    bool CheckPwd(const std::string &user, const std::string &password,
                  UserInfo &userInfo);
    bool AddFriendApply(const std::string &fromUid, const std::string &toUid);
    std::shared_ptr<UserInfo> GetUser(int uid);
    std::vector<std::shared_ptr<UserInfo>> GetUser(const std::string &name);
    bool GetFriendApplyList(const std::string &uid,
                            std::vector<std::shared_ptr<UserInfo>> &applyList);
    bool CheckApplied(const std::string &fromUid, const std::string &toUid);
    bool ChangeMessageStatus(const std::string &uid, int status);
    bool ChangeApplyStatus(const std::string &fromUid, const std::string &toUid,
                           int status);
    bool MakeFriends(const std::string &fromUid, const std::string &toUid);
    bool CheckIsFriend(const std::string &fromUid, const std::string &toUid);
    bool AddNotification(const std::string &uid, int type,
                         const std::string &message);
    bool GetNotificationList(
        const std::string &uid,
        std::vector<std::shared_ptr<UserInfo>> &notificationList);
    bool GetFriendList(const std::string &uid,
                       std::vector<std::shared_ptr<UserInfo>> &friendList);
    bool AddMessage(const std::string &uid, int from_uid, int to_uid,
                    const std::string &timestamp, int env, int content_type,
                    const std::string &content_data,
                    const std::string &content_mime_type,
                    const std::string &fid, int status);
    bool AddConversation(const std::string &uid, int from_uid, int to_uid,
                         const std::string &create_time,
                         const std::string &update_time,
                         const std::string &name, const std::string &icon,
                         int staus, int deleted, int pined, bool processed);
    bool
    GetSeessionList(const std::string &uid,
                    std::vector<std::shared_ptr<SessionInfo>> &sessionList);
    bool GetUnreadMessages(
        const std::string &uid,
        std::vector<std::shared_ptr<im::MessageItem>> &unreadMessages);
    bool ChangeProfile(int uid, int sex, const std::string &name,
                       const std::string &email, const std::string &avatar,
                       const std::string &desc);

    std::string ValueOrEmpty(std::string value);

  private:
    std::unique_ptr<MysqlPool> _pool;
};

#endif
