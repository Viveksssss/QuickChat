#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <memory>
#include <QPixmap>
#include "Properties/singleton.h"

enum class MessageEnv;
struct ConversationItem;
class UserManager
    : public QObject
    , public Singleton<UserManager>
    , public std::enable_shared_from_this<UserManager>
{
    Q_OBJECT
    friend class Singleton<UserManager>;
  public:
    void setupConnections();

    QString pixmapToBase64(const QPixmap& pixmap, const QString& format = "PNG");


    ~UserManager() = default;
    // self
    void SetUid(int)noexcept;
    void SetStatus(int status = 1)noexcept;
    void SetSex(int sex)noexcept;
    void SetName(const QString&name = "卡皮巴拉")noexcept;
    void SetToken(const QString&)noexcept;
    void SetEmail(const QString&)noexcept;
    void SetAvatar(const QPixmap&)noexcept;
    void SetDesc(const QString&)noexcept;
    void SetEnv(const MessageEnv&)noexcept;
    void SetBaseInfo(std::shared_ptr<UserInfo>)noexcept;
    void SetIcon(const QString&)noexcept;



    int GetUid()noexcept;
    int GetStatus()noexcept;
    int GetSex()noexcept;
    QString GetName()noexcept;
    QString GetToken()noexcept;
    QString GetEmail()noexcept;
    QPixmap GetAvatar()noexcept;
    QString GetDesc()noexcept;
    QString GetIcon()noexcept;
    MessageEnv GetEnv()noexcept;
    std::shared_ptr<UserInfo> GetUserInfo();

           // peer
    void SetPeerUid(int)noexcept;
    void SetPeerSex(int sex)noexcept;
    void SetPeerStatus(int status = 1)noexcept;
    void SetPeerName(const QString&)noexcept;
    void SetPeerToken(const QString&)noexcept;
    void SetPeerEmail(const QString&)noexcept;
    void SetPeerAvatar(const QPixmap&)noexcept;
    void SetPeerDesc(const QString&)noexcept;
    void SetPeerIcon(const QString&)noexcept;

    int GetPeerUid()noexcept;
    int GetPeerSex()noexcept;
    int GetPeerStatus()noexcept;
    QString GetPeerName()noexcept;
    QString GetPeerToken()noexcept;
    QString GetPeerEmail()noexcept;
    QPixmap GetPeerAvatar()noexcept;
    QString GetPeerDesc()noexcept;
    QString GetPeerIcon()noexcept;
    std::shared_ptr<UserInfo> GetPeerUserInfo();


    std::vector<std::shared_ptr<UserInfo>>&GetFriends();
    std::vector<std::shared_ptr<ConversationItem>>&GetMessages();
    std::unordered_map<int,QDateTime>&GetTimestamp();

    std::vector<std::shared_ptr<UserInfo>>GetFriendsPerPage(int size = 20);
    std::vector<std::shared_ptr<ConversationItem>>GetMessagesPerPage(int size = 20);

    bool ChangeUserInfo(int peerUid);

    bool IsLoadFriendsFinished();
    bool IsLoadMessagesFinished();
    void ResetLoadFriends();
    void ResetLoadMessages();
    bool IsLoadMessagesFinished(int peerUid); // history
    void setMessagesFinished(int peerUid);
    void addMessagesLoaded(int size);
    QDateTime GetHistoryTimestamp(int);
    void setHistoryTimestamp(int,QDateTime);
    void AddFriendToList(std::shared_ptr<UserInfo>info);
    void AddMessageToList(std::shared_ptr<ConversationItem>info);
  private:
    UserManager();

  private:
    QString _token;
    QString _name;
    QString _email;
    QPixmap _avatar;
    QString _desc;
    QString _icon;  // base64
    int _uid = -1;
    int _sex = -1;
    int _status = -1;
    MessageEnv _env ;

    QString _peer_token;
    QString _peer_name;
    QString _peer_email;
    QPixmap _peer_avatar;
    QString _peer_desc;
    QString _peer_icon;
    int _peer_uid = -1;
    int _peer_sex = -1;
    int _peer_status = -1;

    int _messages_loaded;
    int _friends_loaded;

    std::vector<std::shared_ptr<UserInfo>>_friends;
    std::vector<std::shared_ptr<ConversationItem>>_messages;
    std::unordered_map<int,QDateTime>_timestamp;
    std::unordered_map<int,bool>_messages_finished;

    qint64 _last_time_change_status;

  public slots:
    void do_change_last_time(int,QDateTime);         // from SignalRouter::on_change_last_time
};

#endif // USERMANAGER_H
