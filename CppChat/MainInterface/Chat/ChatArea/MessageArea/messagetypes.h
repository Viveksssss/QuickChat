#ifndef MESSAGETYPES_H
#define MESSAGETYPES_H


#include <QObject>
#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QVariant>
#include <QUuid>
#include "../../../../usermanager.h"
#include "../../../../proto/im.pb.h"

enum class MessageType{
    TextMessage,
    ImageMessage,
    VideoMessage,
    AudioMessage,
    OtherFileMessage
};

enum class MessageSource{
    Me = 0,
    Peer,   // 单独聊天
};

enum class MessageEnv{
    Private,    // 0
    Group       // 1
};

struct MessageContent{
    MessageType     type;           // 自定义类型
    QVariant        data;           // 如果是文本文件，存放在这里，如果是二进制，此为空。
    QString         mimeType;       // 具体的类型比如text/plain
    QString         fid;            // 文件服务器需要用
};

struct MessageItem{
    QString               id;           // 唯一的消息id
    int                   to_id;        // 接受者id
    int                   from_id;      // 发送者的id
    QDateTime             timestamp;    // 时间
    MessageEnv            env;          // 私聊还是群聊
    MessageContent        content;      // 实际的内容串
    bool                  isSelected;   // 之后可能会有聊天记录的选择，删除
    int                   status;

    MessageItem()
        :id(QUuid::createUuid().toString())
        ,from_id(UserManager::GetInstance()->GetUid())
        ,timestamp(QDateTime::currentDateTime())
        ,env(MessageEnv::Private)
        ,isSelected(false)
        ,status(0)
        {}
};

Q_DECLARE_METATYPE(MessageItem)

// 转成发给服务器的im::MessageItem
static im::MessageItem toPb(const MessageItem &m)
{
    im::MessageItem pb;
    pb.set_id(m.id.toStdString());
    pb.set_from_id(m.from_id);
    pb.set_to_id(m.to_id);
    pb.set_timestamp(m.timestamp.toString("yyyy-MM-dd HH:mm:ss").toStdString());
    pb.set_env(static_cast<int32_t>(m.env));


    auto* c = pb.mutable_content();
    c->set_type(static_cast<int32_t>(m.content.type));
    c->set_data(m.content.data.toString().toStdString());
    c->set_mime_type(m.content.mimeType.toStdString());
    c->set_fid(m.content.fid.toStdString());
    return pb;
}

// 服务器收回来解析成MessageItem
static MessageItem fromPb(const im::MessageItem&pb)
{
    QString format = "yyyy-MM-dd HH:mm:ss";
    MessageItem m;
    m.id                = QString::fromStdString(pb.id());
    m.to_id             = pb.to_id();
    m.from_id           = pb.from_id();
    m.timestamp         = QDateTime::fromString(QString::fromStdString(pb.timestamp()),format);
    m.env               = MessageEnv(pb.env());
    m.content.fid       = QString::fromStdString(pb.content().fid());
    m.content.type      = MessageType(pb.content().type());
    m.content.data      = QString::fromStdString(pb.content().data());
    m.content.mimeType  = QString::fromStdString(pb.content().mime_type());
    return m;
}
/*

        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        to_uid      INTEGER NOT NULL UNIQUE,
        from_uid    INTEGER NOT NULL,
        create_time INTEGER NOT NULL,
        update_time INTEGER NOT NULL,
        name        TEXT    NOT NULL,
        icon        TEXT    NOT NULL
*/

struct ConversationItem
{
    QString               id;           // 唯一id
    int                   from_uid;     // 自己
    int                   to_uid;       // 对方id
    QDateTime             create_time;  // 创建时间
    QDateTime             update_time;  // 更新时间
    QString               name;         // 名称
    QString               icon;         // 头像
    int                   status;       // 在线状态
    int                   deleted;      // 是否删除
    int                   pined;        // 是否置顶
    QString               message;      // 最近消息
    bool                  processed;    // 是否处理了
    int                   env;          // 0私聊，1群聊

    ConversationItem()
        : id (QUuid::createUuid().toString())
        , status(0)
        , deleted(0)
        , pined(0)
        , processed(true)
        , env(0)
    {}

};

Q_DECLARE_METATYPE(MessageContent)
Q_DECLARE_METATYPE(QList<MessageContent>)


#endif // MESSAGETYPES_H
