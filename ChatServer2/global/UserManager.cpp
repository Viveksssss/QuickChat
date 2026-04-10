#include "UserManager.h"
#include <mutex>
#include <string>

UserManager::~UserManager()
{
    _uid_with_session.clear();
}
std::shared_ptr<Session> UserManager::GetSession(int uid)
{
    auto uid_str = std::to_string(uid);
    {
        std::lock_guard<std::mutex> lock(_session_mutex);
        auto it = _uid_with_session.find(uid_str);
        if (it == _uid_with_session.end()) {
            return nullptr;
        }
        return it->second;
    }
}
void UserManager::SetUserSession(int uid, std::shared_ptr<Session> session)
{
    auto uid_str = std::to_string(uid);
    {
        std::lock_guard<std::mutex> lock(_session_mutex);
        _uid_with_session[uid_str] = session;
    }
}
void UserManager::RemoveUserSession(int uid)
{
    auto uid_str = std::to_string(uid);
    {
        std::lock_guard<std::mutex> lock(_session_mutex);
        auto it = _uid_with_session.find(uid_str);
        if (it == _uid_with_session.end()) {
            return;
        }
        auto session_id = it->second->GetSessionId();
        if (session_id != uid_str) {
            return;
        }
        _uid_with_session.erase(uid_str);
    }
}
UserManager::UserManager()
{
}
