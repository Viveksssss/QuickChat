#include "MysqlManager.h"
#include "MysqlDao.h"

namespace im {
class MessageItem;
}

MysqlManager::~MysqlManager() {}

bool MysqlManager::test() { return _dao.test(); }

int MysqlManager::TestUidAndEmail(const std::string &uid,
                                  const std::string &email) {
    return _dao.TestUidAndEmail(uid, email);
}

MysqlManager::MysqlManager() {}

int MysqlManager::RegisterUser(const std::string &name,
                               const std::string &email,
                               const std::string &password) {
    return _dao.RegisterUser(name, email, password);
}

int MysqlManager::ResetPassword(const std::string &email,
                                const std::string &password) {
    return _dao.ResetPassword(email, password);
}

bool MysqlManager::CheckPwd(const std::string &user,
                            const std::string &password, UserInfo &userInfo) {
    return _dao.CheckPwd(user, password, userInfo);
}

bool MysqlManager::AddFriendApply(const std::string &fromUid,
                                  const std::string &toUid) {
    return _dao.AddFriendApply(fromUid, toUid);
}

std::shared_ptr<UserInfo> MysqlManager::GetUser(int uid) {
    return _dao.GetUser(uid);
}
std::vector<std::shared_ptr<UserInfo>>
MysqlManager::GetUser(const std::string &name) {
    return _dao.GetUser(name);
}

bool MysqlManager::GetFriendApplyList(
    const std::string &uid, std::vector<std::shared_ptr<UserInfo>> &applyList) {
    return _dao.GetFriendApplyList(uid, applyList);
}

bool MysqlManager::CheckApplied(const std::string &fromUid,
                                const std::string &toUid) {
    return _dao.CheckApplied(fromUid, toUid);
}

bool MysqlManager::ChangeApplyStatus(const std::string &fromUid,
                                     const std::string &toUid, int status) {
    return _dao.ChangeApplyStatus(fromUid, toUid, status);
}

bool MysqlManager::ChangeMessageStatus(const std::string &uid, int status) {
    return _dao.ChangeMessageStatus(uid, status);
}

bool MysqlManager::MakeFriends(const std::string &fromUid,
                               const std::string &toUid) {
    return _dao.MakeFriends(fromUid, toUid);
}

bool MysqlManager::CheckIsFriend(const std::string &fromUid,
                                 const std::string &toUid) {
    return _dao.CheckIsFriend(fromUid, toUid);
}

bool MysqlManager::AddNotification(const std::string &uid, int type,
                                   const std::string &message) {
    return _dao.AddNotification(uid, type, message);
}

bool MysqlManager::GetNotificationList(
    const std::string &uid,
    std::vector<std::shared_ptr<UserInfo>> &notificationList) {
    return _dao.GetNotificationList(uid, notificationList);
}

bool MysqlManager::GetFriendList(
    const std::string &uid,
    std::vector<std::shared_ptr<UserInfo>> &friendList) {
    return _dao.GetFriendList(uid, friendList);
}

bool MysqlManager::AddMessage(const std::string &uid, int from_uid, int to_uid,
                              const std::string &timestamp, int env,
                              int content_type, const std::string &content_data,
                              const std::string &content_mime_type,
                              const std::string &fid, int status) {
    return _dao.AddMessage(uid, from_uid, to_uid, timestamp, env, content_type,
                           content_data, content_mime_type, fid, status);
}

bool MysqlManager::AddConversation(const std::string &uid, int from_uid,
                                   int to_uid, const std::string &create_time,
                                   const std::string &update_time,
                                   const std::string &name,
                                   const std::string &icon, int staus,
                                   int deleted, int pined,
                                   bool processed = false) {
    return _dao.AddConversation(uid, from_uid, to_uid, create_time, update_time,
                                name, icon, staus, deleted, pined, processed);
}

bool MysqlManager::GetSeessionList(
    const std::string &uid,
    std::vector<std::shared_ptr<SessionInfo>> &sessionList) {
    return _dao.GetSeessionList(uid, sessionList);
}

bool MysqlManager::GetUnreadMessages(
    const std::string &uid,
    std::vector<std::shared_ptr<im::MessageItem>> &unreadMessages) {
    return _dao.GetUnreadMessages(uid, unreadMessages);
}

bool MysqlManager::ChangeProfile(int uid, int sex, const std::string &name,
                                 const std::string &email,
                                 const std::string &avatar,
                                 const std::string &desc) {
    return _dao.ChangeProfile(uid, sex, name, email, avatar, desc);
}
