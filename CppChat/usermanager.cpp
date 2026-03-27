#include "usermanager.h"
#include "MainInterface/Chat/ChatArea/MessageArea/messagetypes.h"
#include "../Properties/signalrouter.h"
#include <QBuffer>
#include <QJsonObject>
#include <span>
#include <../../../../tcpmanager.h>


void UserManager::SetName(const QString &name)noexcept
{
    this->_name = name;
}

void UserManager::SetToken(const QString &token)noexcept
{
    this->_token = token;
}

void UserManager::SetUid(int uid) noexcept
{
    this->_uid = uid;
}

void UserManager::SetStatus(int status) noexcept
{
    qint64 now = QDateTime::currentSecsSinceEpoch();

    if (now - _last_time_change_status > 10){
        _last_time_change_status = now;
        this->_status = status;
        QJsonObject obj;
        obj["uid"] = _uid;
        obj["status"] = _status;
        QJsonDocument doc(obj);
        emit TcpManager::GetInstance()->on_send_data(RequestType::ID_SYNC_STATUS,doc.toJson(QJsonDocument::Compact));
    }else{
        qDebug() << "no";
    }
}

void UserManager::SetEmail(const QString &email) noexcept
{
    this->_email = email;
}

void UserManager::SetAvatar(const QPixmap &avatar) noexcept
{
    this->_avatar = avatar;
    QString base64 = pixmapToBase64(avatar);
    this->_icon = base64;
}


void UserManager::SetSex(int sex) noexcept
{
    this->_sex = sex;
}

void UserManager::SetDesc(const QString &desc) noexcept
{
    this->_desc = desc;
}

void UserManager::SetEnv(const MessageEnv &env) noexcept
{
    this->_env = env;
}

void UserManager::SetBaseInfo(std::shared_ptr<UserInfo>info) noexcept
{
    // this->_uid = info->id;
    this->_email = info->email;
    this->_desc = info->desc;
    this->_sex = info->status;
    this->_name = info->name;
    this->_icon = info->avatar;
    SetIcon(info->avatar);
}

void UserManager::SetIcon(const QString &icon) noexcept
{
    _icon =(icon);
    QPixmap pix;
    pix.loadFromData(QByteArray::fromBase64(_icon.toUtf8()));
    this->_avatar = std::move(pix);
}


int UserManager::GetUid() noexcept
{
    return this->_uid;
}

int UserManager::GetStatus() noexcept
{
    return this->_status;
}

QString UserManager::GetName() noexcept
{
    return this->_name;
}

QString UserManager::GetToken() noexcept
{
    return this->_token;
}

QString UserManager::GetEmail() noexcept
{
    return this->_email;
}

QPixmap UserManager::GetAvatar() noexcept
{
    return this->_avatar.isNull()? QPixmap(":/Resources/main/header-default.png") : this->_avatar ;
}

int UserManager::GetSex() noexcept
{
    return this->_sex;
}

QString UserManager::GetDesc() noexcept
{
    return this->_desc;
}

QString UserManager::GetIcon() noexcept
{
    return this->_icon.isEmpty()?":/Resources/main/header-default.png":this->_icon;
}

MessageEnv UserManager::GetEnv() noexcept
{
    return this->_env;
}

std::shared_ptr<UserInfo> UserManager::GetUserInfo()
{
    auto info = std::make_shared<UserInfo>();
    // info->avatar = _icon;
    info->id = _uid;
    info->name = _name;
    info->desc = _desc;
    info->sex = _sex;
    info->status = _status;
    info->avatar = _icon;
    info->email = _email;
    return info;
}

void UserManager::SetPeerName(const QString &name) noexcept
{
    this->_peer_name = name;
}

void UserManager::SetPeerToken(const QString &token) noexcept
{
    this->_peer_token = token;
}

void UserManager::SetPeerUid(int uid) noexcept
{
    this->_peer_uid = uid;
}

void UserManager::SetPeerEmail(const QString &email) noexcept
{
    this->_peer_email = email;
}

void UserManager::SetPeerAvatar(const QPixmap &avatar) noexcept
{
    this->_peer_avatar = avatar;
    QString base64 = pixmapToBase64(avatar);
    this->_peer_icon = base64;
}

void UserManager::SetPeerSex(int sex) noexcept
{
    this->_peer_sex = sex;
}

void UserManager::SetPeerStatus(int status) noexcept
{
    this->_peer_status = status;
}

void UserManager::SetPeerDesc(const QString &desc) noexcept
{
    this->_peer_desc = desc;
}

void UserManager::SetPeerIcon(const QString &icon) noexcept
{
    _peer_icon = (icon);
    QPixmap pix;
    pix.loadFromData(QByteArray::fromBase64(_peer_icon.toUtf8()));
    this->_peer_avatar = std::move(pix);
}


int UserManager::GetPeerUid() noexcept
{
    return this->_peer_uid;
}

QString UserManager::GetPeerName() noexcept
{
    return this->_peer_name;
}

QString UserManager::GetPeerToken() noexcept
{
    return this->_peer_token;
}

QString UserManager::GetPeerEmail() noexcept
{
    return this->_peer_email;
}

QPixmap UserManager::GetPeerAvatar() noexcept
{
    return this->_peer_icon.isEmpty()? QPixmap(":/Resources/main/header-default.png") : this->_peer_avatar ;
}

int UserManager::GetPeerSex() noexcept
{
    return this->_peer_sex;
}

int UserManager::GetPeerStatus() noexcept
{
    return this->_peer_status;
}

QString UserManager::GetPeerDesc() noexcept
{
    return this->_peer_desc;
}

QString UserManager::GetPeerIcon() noexcept
{
    return this->_peer_icon;
}

std::shared_ptr<UserInfo> UserManager::GetPeerUserInfo()
{
    auto info = std::make_shared<UserInfo>();
    info->avatar = _peer_icon;
    info->id = _peer_uid;
    info->name = _peer_name;
    info->desc = _peer_desc;
    info->sex = _peer_sex;
    info->status = _peer_status;
    return info;
}


std::vector<std::shared_ptr<UserInfo> > &UserManager::GetFriends()
{
    return this->_friends;
}

std::vector<std::shared_ptr<ConversationItem> > &UserManager::GetMessages()
{
    return this->_messages;
}

std::unordered_map<int, QDateTime> &UserManager::GetTimestamp()
{
    return this->_timestamp;
}

std::vector<std::shared_ptr<UserInfo> > UserManager::GetFriendsPerPage(int size)
{
    if (size <= 0 || _friends_loaded >= _friends.size()) {
        return {};
    }
    int begin = _friends_loaded;
    int available =  _friends.size() - begin;
    int count  = std::min(size,available);
    _friends_loaded+=count;

    return {_friends.begin() + begin, _friends.begin() + begin + count};
}

std::vector<std::shared_ptr<ConversationItem>>  UserManager::GetMessagesPerPage(int size)
{
    if (size <= 0 || _messages_loaded >= _messages.size()) {
        return {};
    }

    int begin = _messages_loaded;
    int available =  _messages.size() - begin;
    int count  = std::min(size,available);
    _messages_loaded+=count;

    return {_messages.begin() + begin, _messages.begin() + begin + count};
}

bool UserManager::ChangeUserInfo(int peerUid)
{
    auto&friends = this->_friends;
    // Is Friend
    auto it = std::find_if(friends.begin(),friends.end(),[peerUid](std::shared_ptr<UserInfo> info){
        return info->id == peerUid;
    });

    if (it!=friends.end()){
        UserManager::GetInstance()->SetPeerEmail((*it)->email);
        UserManager::GetInstance()->SetPeerDesc((*it)->desc);
        UserManager::GetInstance()->SetPeerSex((*it)->sex);
        UserManager::GetInstance()->SetPeerStatus((*it)->status);
        UserManager::GetInstance()->SetEnv(MessageEnv::Private);
        UserManager::GetInstance()->GetTimestamp().erase(UserManager::GetInstance()->GetPeerUid());
        UserManager::GetInstance()->SetPeerName((*it)->name);
        UserManager::GetInstance()->SetPeerUid(peerUid);
        UserManager::GetInstance()->SetPeerIcon((*it)->avatar);
        return true;
    }
    return false;
}

bool UserManager::IsLoadFriendsFinished()
{
    return _friends_loaded>=_friends.size() ? true:false;
}

bool UserManager::IsLoadMessagesFinished()
{
    return _messages_loaded>=_messages.size()?true:false;
}

void UserManager::ResetLoadFriends()
{
    this->_friends_loaded = 0;
}

void UserManager::ResetLoadMessages()
{
    this->_messages_loaded = 0;
}

bool UserManager::IsLoadMessagesFinished(int peerUid)
{
    auto it = _messages_finished.find(peerUid);
    if (it == _messages_finished.end()){
        _messages_finished[peerUid] = false;
        return _messages_finished[peerUid];
    }else{
        return it->second;
    }
}

void UserManager::setMessagesFinished(int peerUid)
{
    _messages_finished[peerUid] = true;
}

void UserManager::addMessagesLoaded(int size)
{
    this->_messages_loaded++;
}

QDateTime UserManager::GetHistoryTimestamp(int peerUid)
{
    auto it =_timestamp.find(peerUid);
    if (it == _timestamp.end()){
        _timestamp[peerUid] = QDateTime::currentDateTime();
        return _timestamp[peerUid];
    }else{
        return it->second;
    }
}

void UserManager::setHistoryTimestamp(int peerUid, QDateTime time)
{
    _timestamp[peerUid] = time;
}

void UserManager::AddFriendToList(std::shared_ptr<UserInfo> info)
{
    this->_friends.push_back(info);
}

void UserManager::AddMessageToList(std::shared_ptr<ConversationItem> info)
{
    this->_messages.push_back(info);
}

UserManager::UserManager()
    : _name("")
    , _token("")
    , _uid(-1)
    , _messages_loaded(0)
    , _friends_loaded(0)
    , _env(MessageEnv::Private)
    , _last_time_change_status(QDateTime::currentSecsSinceEpoch())
{
    setupConnections();
}

void UserManager::do_change_last_time(int uid, QDateTime time)
{
    _timestamp[uid] = time;
}

void UserManager::setupConnections()
{
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_change_last_time,this,&UserManager::do_change_last_time);
}

QString UserManager::pixmapToBase64(const QPixmap& pixmap, const QString& format) {
    if (pixmap.isNull()) {
        return "";
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

           // 保存图片数据到 buffer
    bool success = pixmap.save(&buffer, format.toUtf8().constData());
    if (!success) {
        return "";
    }

           // 转换为 Base64
    QString base64 = QString::fromLatin1(byteArray.toBase64());
    return base64;
}
