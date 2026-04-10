#ifndef MESSAGESMODEL_H
#define MESSAGESMODEL_H

#include "../../Chat/ChatArea/MessageArea/messagetypes.h"

#include <QAbstractListModel>
#include <QObject>

struct ConversationItem;
class MessagesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MessageRole{
        IdRole = Qt::UserRole + 1,  // id
        ToUidRole,
        NameRole,                   // 昵称
        AvatarRole,                 // 头像
        StatusRole,                 // 状态
        MessageRole,                // 最近消息
        RedDotRole,                 // 红点
        MessageEnvRole              // 私聊/群聊
    };

    explicit MessagesModel(QObject *parent = nullptr);
    ~MessagesModel();
    // QAbstractItemModel interface
    int rowCount(const QModelIndex&parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int,QByteArray>roleNames()const override;

    void addMessage(const ConversationItem&messageItem);
    void addPreMessage(const ConversationItem&messageItem);
    ConversationItem getMessage(int index);

    ConversationItem getConversation(int to_uid);

    // 在 MessagesModel 类中添加方法
    QModelIndex indexFromUid(int uid) const;

    QVector<ConversationItem>&getList();

    // 根据uid查找消息
    bool existMessage(int uid);
    // 处理信息存储到数据库
    void processPendingUpdates();

private:
    QVector<ConversationItem>_messages;
    QSet<int>_pendingUpdates;

    // QAbstractItemModel interface
public:
    bool removeRows(int row, int count, const QModelIndex &parent)override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)override;
    bool setData(const QModelIndex &index, const QVariant &value, int role)override;
};

#endif // MESSAGESMODEL_H
