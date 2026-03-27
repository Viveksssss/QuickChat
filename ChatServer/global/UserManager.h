#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "../session/Session.h"
#include "./Singleton.h"
#include <memory>
#include <mutex>
#include <unordered_map>

class UserManager : public Singleton<UserManager> {
    friend class Singleton<UserManager>;

public:
    ~UserManager();
    std::shared_ptr<Session> GetSession(int uid);
    void SetUserSession(int uid, std::shared_ptr<Session> session);
    void RemoveUserSession(int uid);

private:
    UserManager();
    std::mutex _session_mutex;
    std::unordered_map<std::string, std::shared_ptr<Session>> _uid_with_session;
};

#endif
