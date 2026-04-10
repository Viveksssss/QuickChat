#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <functional>
#include <QMetaType>

class QWidget;
enum class RequestType{
    ID_GET_VARIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //注册用户
    ID_RESET_PWD = 1003, //重置密码
    ID_LOGIN_USER = 1004, //用户登录
    ID_CHAT_LOGIN = 1005, //登陆聊天服务器
    ID_CHAT_LOGIN_RSP= 1006, //登陆聊天服务器回包
    ID_SEARCH_USER_REQ = 1007, //用户搜索请求
    ID_SEARCH_USER_RSP = 1008, //搜索用户回包
    ID_ADD_FRIEND_REQ = 1009,  //添加好友申请
    ID_ADD_FRIEND_RSP = 1010, //申请添加好友回复
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //通知用户添加好友申请
    ID_AUTH_FRIEND_REQ = 1013,  //认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,  //认证好友回复
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //通知用户认证好友申请
    ID_TEXT_CHAT_MSG_REQ  = 1017,  //文本聊天信息请求
    ID_TEXT_CHAT_MSG_RSP  = 1018,  //文本聊天信息回复
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //通知用户文本聊天信息
    ID_NOTIFY_OFF_LINE_REQ = 1021, //通知用户下线
    ID_HEART_BEAT_REQ = 1023,      //心跳请求
    ID_HEARTBEAT_RSP = 1024,       //心跳回复
    ID_NOTIFY = 1025, // 通知用户建立好友关系
    ID_GET_MESSAGES_OF_FRIEND_REQ = 1026, // 获取与某个好友的信息请求
    ID_GET_MESSAGES_OF_FRIEND_RSP = 1027, // 获取消息回包
    ID_SYNC_CONVERSATIONS_REQ = 1028,          // 同步会话
    ID_SYNC_PERSONAL_INFORMATION_REQ = 1029,    // 更新个人信息
    ID_SYNC_PERSONAL_INFORMATION_RSP = 1030, // 更新个人信息回包
    ID_SYNC_STATUS = 1031,  // 更新状态
    ID_SYNC_STATUS_RSP = 1032,               // 更新状态回包
    ID_SYNC_INFORMATIONS = 1033,             // 重新上线获取未读信息
};

Q_DECLARE_METATYPE(RequestType)

enum class Modules{
    REGISTERMOD = 0,
    FORGOTMOD = 1,
    LOGINMOD = 2
};

enum class ErrorCodes {
    SUCCESS = 0,
    ERROR_NETWORK = 1001,
    ERROR_JSON = 1002,
    RPCFAILED = 1003,
    ERROR_SECURITYCODE_EXPIRED = 1004,
    ERROR_SECURITYCODE_NOTFOUND = 1005,
    ERROR_EMAIL_NOTFOUND = 1006,
    ERROR_USER_OR_PASSWORD_INCORRECT = 1007
};

enum class RegisterVarify{
    ERROR_CONTENT_INCOMPLETE,
    ERROR_EMAIL_INCORRECTFORMAT,
    ERROR_PASSWORD_NOTSURE,
    ERROR_PASSWORD_LEN,
    ERROR_SECURITY_EMPTY,
    SUCCESS
};

enum class NotificationCodes {
    ID_NOTIFY_MAKE_FRIENDS = 1001,
    ID_NOTIFY_NOT_FRIENDs = 1002,
    ID_NOTIFY_FRIEND_ONLINE = 1003,
};

struct ServerInfo{
    QString host;
    QString port;
    QString token;
    QString email;
    QString name;
    int uid;
};

//user->id,user->avatar,user->name,user->sex,user->status,user->isFriend
struct FriendInfo
{
    int id;
    int status;
    int sex;
    bool isFriend;
    QString name;
    QString avatar;
    QString desc;
    FriendInfo(int id,const QString&avatar,const QString&name,int sex,int status,bool isFriend)
        : id (id)
        , avatar(avatar)
        , name(name)
        , sex(sex)
        , status(status)
        , isFriend(isFriend)
    {}
};

Q_DECLARE_METATYPE(FriendInfo)

struct UserInfo{
    int id;
    int sex;
    int status;
    QString email;
    QString name;
    QString avatar;
    QString desc;
    QString back;   // 备用字段

    UserInfo(){}
    UserInfo(int id,const QString&name,const QString&avatar,int status)
        : id(id)
        , name(name)
        , avatar(avatar)
        , status(status)
    {}
    UserInfo(int id,int status,int sex,const QString&name,const QString&avatar,const QString&email)
        : id(id)
        , status(status)
        , sex(sex)
        , name(name)
        , avatar(avatar)
        , email(email)
    {}
    UserInfo(int id,int status,int sex,const QString&email,const QString&name,const QString&avatar,QString desc)
        : id(id)
        , email(email)
        , name(name)
        , avatar(avatar)
        , status(status)
        , desc(desc)
    {}
};

Q_DECLARE_METATYPE(UserInfo)

class Defer {
public:
    Defer(std::function<void()> func);
    ~Defer();

private:
    std::function<void()> m_func;
};

extern QString gate_url_prefix;

extern std::function<QString(QString)>cryptoString;

extern void showToolTip(QWidget *widget,const QString&str,int yOffset = 0);


#endif // GLOBAL_H
