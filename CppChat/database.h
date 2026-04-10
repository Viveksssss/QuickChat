#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <vector>
#include <memory>
#include "MainInterface/Chat/ChatArea/MessageArea/messagetypes.h"
#include "../Properties/signalrouter.h"


class DataBase
{
public:
    static DataBase&GetInstance();

    // 聊天记录
    bool initialization(const QString&db_path = "");

    bool createMessagesTables();

    bool storeMessage(const MessageItem&message);

    bool storeMessages(const std::vector<MessageItem>&messages);
    bool storeMessages(const std::vector<std::shared_ptr<MessageItem>>&messages);

    std::vector<MessageItem>getMessages(int peerUid, QString sinceTimestamp = 0,int limit = 20);
    std::optional<MessageItem> getMessage(int peerUid,const QString&message_id);

    bool updateMessageStatus(int messageId,int status);

    bool updateMessagesStatus(int peerUid, int status);

    bool deleteMessage(int messageId);

    MessageItem createMessageFromQuery(const QSqlQuery& query);

    // 会话列表
    bool createConversationTable();

    bool createOrUpdateConversation(const ConversationItem& conv);

    bool existConversation(int peerUid);

    bool createOrUpdateConversations(const std::vector<ConversationItem>&conversations);
    bool createOrUpdateConversations(const std::vector<std::shared_ptr<ConversationItem>>&conversations);

    std::vector<ConversationItem> getConversationList();

    std::vector<std::shared_ptr<ConversationItem>> getConversationListPtr();

    ConversationItem getConversation(int peerUid);

    ConversationItem createConversationFromQuery(const QSqlQuery& query);

    QString getLastMessage(int peerUid);

    // 好友列表
    bool createFriendsTable();

    std::shared_ptr<UserInfo>getFriendInfoPtr(int peerUid);

    UserInfo getFriendInfo(int peerUid);

    std::vector<UserInfo>getFriends();

    std::vector<std::shared_ptr<UserInfo>>getFriendsPtr();

    bool storeFriends(const std::vector<std::shared_ptr<UserInfo>>friends);

    bool storeFriends(const std::vector<UserInfo>friends);

    bool storeFriend(const UserInfo&info);

    bool storeFriend(const std::shared_ptr<UserInfo>&info);

    UserInfo createFriendInfoFromQuery(const QSqlQuery& query);

private:
    DataBase() = default;
private:
    QString _db_path;
    QSqlDatabase _db;
};

#endif // DATABASE_H
