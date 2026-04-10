#include "tcpmanager.h"
#include "MainInterface/Chat/ChatArea/MessageArea/messagetypes.h"
#include "usermanager.h"
#include "database.h"
#include "../proto/im.pb.h"
#include <QAbstractSocket>
#include <QDataStream>
#include <QJsonObject>
#include <QDir>
#include <QJsonArray>
#include <QMessageBox>
#include <future>



TcpManager::~TcpManager() = default;

bool TcpManager::isConnected()
{
    return _socket.state() == QAbstractSocket::ConnectedState;
}

TcpManager::TcpManager()
    : _host("")
    , _port(0)
    , _recv_pending(false)
    , _msg_id(0)
    , _msg_len(0)
    , _ok(false)
    , _pending(false)
    , _bytes_send(0)

{
    RegisterMetaType();
    initHandlers();
    connections();
}

void TcpManager::initHandlers()
{
    /**
     * @brief 用户登录请求回包处理
     */
    _handlers[RequestType::ID_CHAT_LOGIN_RSP] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            qDebug() << "Error occured about Json";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")){
            int err = static_cast<int>(ErrorCodes::ERROR_JSON);
            qDebug() << "Login Failed,Error Is Json Parse Error " <<err;
            emit on_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int>(ErrorCodes::SUCCESS)){
            qDebug() << "Login Failed,Error Is " << err;
            emit on_login_failed(err);
            return;
        }
        // 初始化本地数据库
        DataBase::GetInstance().initialization();

        // 基本信息
        UserManager::GetInstance()->SetName(jsonObj["name"].toString());
        UserManager::GetInstance()->SetEmail(jsonObj["email"].toString());
        UserManager::GetInstance()->SetToken(jsonObj["token"].toString());
        UserManager::GetInstance()->SetIcon(jsonObj["icon"].toString());
        UserManager::GetInstance()->SetUid(jsonObj["uid"].toInt());
        UserManager::GetInstance()->SetSex(jsonObj["sex"].toInt());
        UserManager::GetInstance()->SetDesc(jsonObj["desc"].toString());
        UserManager::GetInstance()->SetIcon(jsonObj["icon"].toString());

        qDebug() <<"icon:" <<jsonObj["icon"].toString();
        UserManager::GetInstance()->SetStatus(1);


        // 先是本地加载
        UserManager::GetInstance()->GetFriends() = DataBase::GetInstance().getFriendsPtr();
        UserManager::GetInstance()->GetMessages() = DataBase::GetInstance().getConversationListPtr();

        // emit on_add_friends_to_list(UserManager::GetInstance()->GetFriendsPerPage());
        // emit on_add_messages_to_list(UserManager::GetInstance()->GetMessagesPerPage());



        // 申请列表
        if (jsonObj.contains("apply_friends")){
            const QJsonArray &apply_friends = jsonObj["apply_friends"].toArray();
            std::vector<std::shared_ptr<UserInfo>>apply_list;
            for(const QJsonValue&value:apply_friends){
                QJsonObject obj = value.toObject();
                auto user_info = std::make_shared<UserInfo>();
                user_info->id = obj["uid"].toInt();
                user_info->name = obj["name"].toString();
                user_info->email = obj["email"].toString();
                user_info->avatar = obj["icon"].toString();
                user_info->sex = obj["sex"].toInt();
                user_info->desc = obj["desc"].toString();
                user_info->back = obj["back"].toString();//备用字段这里存放时间
                apply_list.push_back(user_info);
            }
            emit on_get_apply_list(apply_list);
        }

        // 通知列表
        if(jsonObj.contains("notifications")){
            const QJsonArray &notification_array = jsonObj["notifications"].toArray();
            std::vector<std::shared_ptr<UserInfo>>notification_list;
            for(const QJsonValue&value:notification_array){
                QJsonObject obj = value.toObject();
                auto user_info = std::make_shared<UserInfo>();
                user_info->id = obj["uid"].toInt();
                user_info->status = obj["type"].toInt();
                user_info->desc = obj["message"].toString();
                user_info->back = obj["time"].toString();
                user_info->avatar = obj["icon"].toString();
                notification_list.push_back(user_info);
            }
            emit on_notifications_to_list(notification_list);
        }

        // 好友列表
        if (jsonObj.contains("friends")){
            const QJsonArray &friend_array = jsonObj["friends"].toArray();
            std::vector<std::shared_ptr<UserInfo>>lists;
            for(const QJsonValue&value:friend_array){
                QJsonObject obj = value.toObject();
                auto user_info = std::make_shared<UserInfo>();
                user_info->id = obj["uid"].toInt();
                user_info->sex = obj["sex"].toInt();
                user_info->status = obj["status"].toInt();
                user_info->name = obj["name"].toString();
                user_info->email = obj["email"].toString();
                user_info->avatar = obj["icon"].toString();
                user_info->desc = obj["desc"].toString();
                user_info->back = obj["back"].toString();
                lists.push_back(user_info);
                // UserManager::GetInstance()->GetFriends().push_back(user_info);
            }
            int ss = UserManager::GetInstance()->GetFriends().size();
            if (lists.size()>UserManager::GetInstance()->GetFriends().size()){
                (void)std::async(std::launch::async,[this,&lists](){
                    qDebug() << "friends";
                    DataBase::GetInstance().storeFriends(lists);
                    UserManager::GetInstance()->GetFriends() = std::move(lists);
                    UserManager::GetInstance()->ResetLoadFriends();
                });
            }
        }

        //TODO: 会话列表
        if (jsonObj.contains("conversations")){
            const QJsonArray &conversations = jsonObj["conversations"].toArray();
            qDebug() << "conversations" <<conversations;
            std::vector<std::shared_ptr<ConversationItem>>lists;
            for(const QJsonValue&value:conversations){
                QJsonObject obj = value.toObject();
                auto conversation = std::make_shared<ConversationItem>();
                // 解析字段，仿照好友列表的写法
                conversation->id = obj["uid"].toString();
                conversation->to_uid = obj["to_uid"].toInt();
                conversation->from_uid = obj["from_uid"].toInt();
                conversation->create_time = obj["create_time"].toVariant().toDateTime();
                conversation->update_time = obj["update_time"].toVariant().toDateTime();
                conversation->name = obj["name"].toString();
                conversation->icon = obj["icon"].toString();
                conversation->status = obj["status"].toInt();
                conversation->deleted = obj["deleted"].toInt();
                conversation->pined = obj["pined"].toInt();
                // UserManager::GetInstance()->GetMessages().push_back(conversation);
                lists.push_back(conversation);
            }

            if (UserManager::GetInstance()->GetMessages().size()<lists.size()){
                (void)std::async(std::launch::async,[this,&lists](){
                    DataBase::GetInstance().createOrUpdateConversations(lists);
                    UserManager::GetInstance()->GetMessages() = std::move(lists);
                    UserManager::GetInstance()->ResetLoadMessages();
                });
            }
        }

        // 解析消息列表
        if (jsonObj.contains("unread_messages")){
            const QJsonArray&unread_messages = jsonObj["unread_messages"].toArray();
            std::vector<std::shared_ptr<MessageItem>>lists;
            for (const QJsonValue&value:unread_messages){
                QJsonObject obj = value.toObject();
                auto message = std::make_shared<MessageItem>();
                message->id = obj.value("id").toVariant().toString();
                message->from_id = obj.value("from_id").toInt();
                message->to_id = obj.value("to_id").toInt();
                message->timestamp =QDateTime::fromString(obj.value("timestamp").toString(),"yyyy-MM-dd hh:mm:ss");
                message->env = MessageEnv(obj.value("env").toInt());
                message->content.type = MessageType(obj.value("content_type").toInt());
                message->content.data = obj.value("content_data").toString();
                message->content.mimeType = obj.value("content_mime_type").toString();
                message->content.fid = obj.value("content_fid").toString();
                lists.push_back(message);
            }
            emit on_get_messages(lists);
        }
        // 发出信号跳转到主页面
        _ok = true;
        emit on_switch_interface();
    };
    /**
     * @brief 用户搜索回包处理
     */
    _handlers[RequestType::ID_SEARCH_USER_RSP] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            qDebug() << "Error occured about Json";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")){
            int err = static_cast<int>(ErrorCodes::ERROR_JSON);
            qDebug() << "Search Failed,Error Is Json Parse Error " <<err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int>(ErrorCodes::SUCCESS)){
            qDebug() << "Search Failed,Error Is " << err;
            return;
        }

        // 解析查询的用户列表
        QList<std::shared_ptr<FriendInfo>> userList;
        if (jsonObj.contains("users") && jsonObj["users"].isArray()) {
            const QJsonArray &usersArray = jsonObj["users"].toArray();

            for (const QJsonValue &userValue : usersArray) {
                if (userValue.isObject()) {
                    QJsonObject userObj = userValue.toObject();

                    QString avatar;
                    if (userObj.contains("icon")&&userObj["icon"]!="NULL"&&!userObj["icon"].isNull()) {
                        avatar = userObj["icon"].toString();
                    } else {
                        // 没有头像字段，使用默认头像
                        avatar = ":/Resources/main/header-default.png";
                    }

                    int id = userObj["uid"].toInt();
                    int status = userObj["status"].toInt();
                    int sex = userObj["sex"].toInt();
                    // QString email = userObj["email"].toString();
                    QString name = userObj["name"].toString();
                    bool isFriend = userObj["isFriend"].toBool();
                    auto user_info = std::make_shared<FriendInfo>(id,avatar,name,sex,status,isFriend);

                    userList.append(user_info);
                }
            }

        }
        if(userList.count() == 0){
            return;
        }
        emit on_users_searched(userList);
    };

    /**
     * @brief 用户添加请求回包处理
     */
    _handlers[RequestType::ID_ADD_FRIEND_RSP] = [this](RequestType requestType,int len,QByteArray data){
      QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
      if (jsonDoc.isNull()){
          qDebug() << "Error occured about Json";
          return;
      }
      QJsonObject jsonObj = jsonDoc.object();
      if (!jsonObj.contains("error")){
          int err = static_cast<int>(ErrorCodes::ERROR_JSON);
          qDebug() << "AddFriend Failed,Error Is Json Parse Error " <<err;
          return;
      }

      int err = jsonObj["error"].toInt();
      if (err != static_cast<int>(ErrorCodes::SUCCESS)){
          qDebug() << "AddFriend Failed,Error Is " << err;
          return;
      }
      UserInfo info;
      info.id = jsonObj["fromUid"].toInt();
      // TODO:
      qDebug() << "申请添加好友成功";
      emit on_add_friend(info);
  };

    /**
     * @brief 用户请求添加好友通知处理
     */
    _handlers[RequestType::ID_NOTIFY_ADD_FRIEND_REQ] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")){
            int err = static_cast<int>(ErrorCodes::ERROR_JSON);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int>(ErrorCodes::SUCCESS)){
            return;
        }

        int from_uid = jsonObj["from_uid"].toInt();
        int from_sex = jsonObj["from_sex"].toInt();
        QString from_name = jsonObj["from_name"].toString();
        QString from_icon = jsonObj["from_icon"].toString();
        QString from_desc = jsonObj["from_desc"].toString();

        auto user_info = std::make_shared<UserInfo>();
        user_info->id = from_uid;
        user_info->sex = from_sex;
        user_info->name = from_name;
        user_info->avatar = from_icon;
        user_info->desc = from_desc;

        emit on_auth_friend(user_info);
    };

    /**
     * @brief 用户请求添加好友通知回包
     */
    _handlers[RequestType::ID_AUTH_FRIEND_RSP] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")){
            int err = static_cast<int>(ErrorCodes::ERROR_JSON);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int>(ErrorCodes::SUCCESS)){
            return;
        }
        // 接受信息如果成功，然后将对方信息加入好友列表
        if (jsonObj.contains("ok") && jsonObj["ok"].toBool()){
            auto info = std::make_shared<UserInfo>();
            info->id = jsonObj["to_uid"].toInt();
            info->status = jsonObj["to_status"].toInt();
            info->sex = jsonObj["to_sex"].toInt();
            info->name = jsonObj["to_name"].toString();
            info->email = jsonObj["to_email"].toString();
            info->avatar = jsonObj["to_icon"].toString();
            info->desc = jsonObj["to_desc"].toString();
            info->back = jsonObj["to_message"].toString();
            if (jsonObj["accept"].toBool()){
                emit on_add_friend_to_list(info);
                DataBase::GetInstance().storeFriend(info);
            }
            emit on_notify_friend(info,jsonObj["accept"].toBool());
        }else{
            // 暂时忽略
        }
    };

    _handlers[RequestType::ID_NOTIFY_AUTH_FRIEND_REQ] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")){
            int err = static_cast<int>(ErrorCodes::ERROR_JSON);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int>(ErrorCodes::SUCCESS)){
            return;
        }

        auto info = std::make_shared<UserInfo>();
        info->id = jsonObj["from_uid"].toInt();
        info->name = jsonObj["from_name"].toString();
        info->sex = jsonObj["from_sex"].toInt();
        info->email = jsonObj["from_email"].toString();
        info->avatar = jsonObj["from_icon"].toString();
        info->status = jsonObj["from_status"].toInt();
        info->desc = jsonObj["from_desc"].toString();     // 临时存放消息
        info->back = jsonObj["message"].toString();
        if (jsonObj["type"].toInt() == static_cast<int>(NotificationCodes::ID_NOTIFY_MAKE_FRIENDS)){
            emit on_add_friend_to_list(info);
            emit on_notify_friend2(info,true);
            DataBase::GetInstance().storeFriend(info);
        }else{
            emit on_notify_friend2(info,false);
        }
    };

    /**
     * @brief 通知
    */
    _handlers[RequestType::ID_NOTIFY] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")){
            int err = static_cast<int>(ErrorCodes::ERROR_JSON);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int>(ErrorCodes::SUCCESS)){
            return;
        }

        auto user_info = std::make_shared<UserInfo>();
        user_info->id = jsonDoc["uid"].toInt();
        user_info->status = jsonDoc["type"].toInt();
        user_info->desc = jsonDoc["message"].toString();
        user_info->back = jsonDoc["time"].toString();
        user_info->avatar = jsonDoc["icon"].toString();
        std::vector<std::shared_ptr<UserInfo>>vec;
        vec.push_back(user_info);
        emit on_notifications_to_list(vec);
        emit on_change_friend_status(user_info->id,1);
    };

    /**
     * @brief 收到消息
    */
    _handlers[RequestType::ID_NOTIFY_TEXT_CHAT_MSG_REQ] = [this](RequestType requestType,int len,QByteArray data){
        im::MessageItem pb;
        if (pb.ParseFromString(data.toStdString())){
            emit on_get_message(fromPb(pb));
        }else{
            qDebug() << "Failed to parse Message from data";
        }
    };

    /**
     * @brief 发送消息回包
    */
    _handlers[RequestType::ID_TEXT_CHAT_MSG_RSP] = [this](RequestType requestType,int len,QByteArray data){
        //TODO: qDebug() << "暂时不处理";
    };

    /**
     * @brief 获取与某人的消息记录回报
    */
    _handlers[RequestType::ID_GET_MESSAGES_OF_FRIEND_RSP] = [this](RequestType requestType,int len,QByteArray data){
        // TODO:获取与某人的聊天记录列表
    };
    /**
     * @brief 通知下线了
     */
    _handlers[RequestType::ID_NOTIFY_OFF_LINE_REQ] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error") || jsonObj["error"].toInt()!=static_cast<int>(ErrorCodes::SUCCESS)){
            return;
        }

        if (jsonObj["uid"].toInt() == UserManager::GetInstance()->GetUid()){
            _socket.disconnectFromHost();

            // 弹出对话框提示
            QMessageBox msgBox;
            msgBox.setWindowTitle("连接提示");
            msgBox.setText("您的账号在另一台设备登录，当前连接将被断开。");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);

            // 显示对话框并等待用户确认
            msgBox.exec();

            emit on_switch_login();
        }
    };

    /**
     * @brief 心跳回包
     */
    _handlers[RequestType::ID_HEARTBEAT_RSP] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            emit on_switch_login();
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error") || jsonObj["error"].toInt()!=static_cast<int>(ErrorCodes::SUCCESS)){
            emit on_switch_login();
            return;
        }
        qDebug() << "Received Heartbeat from Server";
    };

    /**
     * @brief 修改资料回包
     */
    _handlers[RequestType::ID_SYNC_PERSONAL_INFORMATION_RSP] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        if (!jsonObj.contains("error") || jsonObj["error"].toInt()!=static_cast<int>(ErrorCodes::SUCCESS)){
            msgBox.setWindowTitle("修改失败");
            msgBox.setText("修改资料异常！");
        }else{
            msgBox.setWindowTitle("修改成功");
            msgBox.setText("修改资料成功！");
        }
        msgBox.exec();
    };


    /**
     * @brief 修改状态回包
     */
    _handlers[RequestType::ID_SYNC_STATUS_RSP] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        if (!jsonObj.contains("error") || jsonObj["error"].toInt()!=static_cast<int>(ErrorCodes::SUCCESS)){
            msgBox.setWindowTitle("状态异常");
            msgBox.setText("状态修改异常！");
            msgBox.exec();
        }
    };

    /**
     * @brief 重新上线获取信息
     */
    _handlers[RequestType::ID_SYNC_INFORMATIONS] = [this](RequestType requestType,int len,QByteArray data){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull()){
            return;
        }

        qDebug() << data;
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject jsonObj= doc.object();
        if (jsonObj["uid"].toInt() != UserManager::GetInstance()->GetUid()){
            return;
        }

        // 申请列表
        if (jsonObj.contains("apply_friends")){
            const QJsonArray &apply_friends = jsonObj["apply_friends"].toArray();
            std::vector<std::shared_ptr<UserInfo>>apply_list;
            for(const QJsonValue&value:apply_friends){
                QJsonObject obj = value.toObject();
                auto user_info = std::make_shared<UserInfo>();
                user_info->id = obj["uid"].toInt();
                user_info->name = obj["name"].toString();
                user_info->email = obj["email"].toString();
                user_info->avatar = obj["icon"].toString();
                user_info->sex = obj["sex"].toInt();
                user_info->desc = obj["desc"].toString();
                user_info->back = obj["back"].toString();//备用字段这里存放时间
                apply_list.push_back(user_info);
            }
            emit on_get_apply_list(apply_list);
        }

        // 通知列表
        if(jsonObj.contains("notifications")){
            const QJsonArray &notification_array = jsonObj["notifications"].toArray();
            std::vector<std::shared_ptr<UserInfo>>notification_list;
            for(const QJsonValue&value:notification_array){
                QJsonObject obj = value.toObject();
                auto user_info = std::make_shared<UserInfo>();
                user_info->id = obj["uid"].toInt();
                user_info->status = obj["type"].toInt();
                user_info->desc = obj["message"].toString();
                user_info->back = obj["time"].toString();
                user_info->avatar = obj["icon"].toString();
                notification_list.push_back(user_info);
            }
            emit on_notifications_to_list(notification_list);
        }

        // 解析消息列表
        if (jsonObj.contains("unread_messages")){
            const QJsonArray&unread_messages = jsonObj["unread_messages"].toArray();
            std::vector<std::shared_ptr<MessageItem>>lists;
            for (const QJsonValue&value:unread_messages){
                QJsonObject obj = value.toObject();
                auto message = std::make_shared<MessageItem>();
                message->id = obj.value("id").toVariant().toString();
                message->from_id = obj.value("from_id").toInt();
                message->to_id = obj.value("to_id").toInt();
                message->timestamp =QDateTime::fromString(obj.value("timestamp").toString(),"yyyy-MM-dd hh:mm:ss");
                message->env = MessageEnv(obj.value("env").toInt());
                message->content.type = MessageType(obj.value("content_type").toInt());
                message->content.data = obj.value("content_data").toString();
                message->content.mimeType = obj.value("content_mime_type").toString();
                message->content.fid = obj.value("content_fid").toString();
                lists.push_back(message);
            }
            emit on_get_messages(lists);
        }
    };

}

void TcpManager::connections()
{
    // 连接成功
    connect(&_socket,&QTcpSocket::connected, this,[&](){
        qDebug() << "Connected to Server " << _socket.peerAddress().toString() << ":" << _socket.peerPort();
        emit on_connect_success(true);
    });

    connect(&_socket, &QTcpSocket::readyRead, this,[&]() {
        _buffer.append(_socket.readAll());
        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);

        qDebug() <<"buffer:" <<_buffer.size();

        while (_buffer.size() >= sizeof(qint16) * 2) {
            // 直接读取消息头
            QDataStream stream(_buffer);
            stream.setByteOrder(QDataStream::BigEndian);
            quint16 msg_id, msg_len;
            stream >> msg_id >> msg_len;
            int total_size = sizeof(quint16) * 2 + msg_len;

            // 检查消息长度有效性
            if (msg_len < 0) {
                qCritical() << "无效的消息长度:" << msg_len << "，清空缓冲区";
                _buffer.clear();
                return;
            }

            // 检查是否有完整消息
            if (_buffer.size() < total_size) {
                break;
            }

            // 读取消息体
            QByteArray msgBody = _buffer.mid(sizeof(quint16) * 2, msg_len);

            // 处理消息
            handleMessage(RequestType(msg_id), msg_len, msgBody);

            // 移除已处理数据
            _buffer.remove(0, total_size);
        }
    });
    // 错误
    connect(&_socket,&QTcpSocket::errorOccurred,this,[&](QTcpSocket::SocketError socketError){
        qDebug() << "Socket Error["<<socketError<< "]:" << _socket.errorString();
        emit on_login_failed(static_cast<int>(ErrorCodes::ERROR_NETWORK));
        // 初始化本地数据库
        DataBase::GetInstance().initialization();

        // 连接网络失败直接使用本地数据库展示。
        UserManager::GetInstance()->GetFriends() = DataBase::GetInstance().getFriendsPtr();
        UserManager::GetInstance()->GetMessages() = DataBase::GetInstance().getConversationListPtr();

        emit on_add_friends_to_list(UserManager::GetInstance()->GetFriendsPerPage());
        emit on_add_messages_to_list(UserManager::GetInstance()->GetMessagesPerPage());
    });
    // 断开连接
    connect(&_socket,&QTcpSocket::disconnected,this,[&](){
        qDebug() << "Disconnected from server - " << _socket.peerAddress().toString() <<":"<<_socket.peerPort();
        emit on_connection_closed();
    });
    // 发送数据
    connect(this,&TcpManager::on_send_data,this,&TcpManager::do_send_data);
    connect(&_socket,&QTcpSocket::bytesWritten,this,[this](quint64 bytes){
        _bytes_send += bytes;
        if (_bytes_send < _current_block.size()){
            auto data_to_send = _current_block.mid(_bytes_send);
            _socket.write(data_to_send);
            return;
        }

        if (_queue.empty()){
            _current_block.clear();
            _pending = false;
            _bytes_send = 0;
            return;
        }

        _current_block = _queue.dequeue();
        _bytes_send = 0;
        _pending = true;
        quint64 w2 = _socket.write(_current_block);

    });
}

void TcpManager::handleMessage(RequestType requestType, int len, QByteArray data)
{
    auto it = _handlers.find(requestType);
    if ( it == _handlers.end()){
        qDebug() <<  "Not Found[" << static_cast<int>(requestType) << "] to handle";
        return;
    }
    it.value()(requestType,len,data);
}

void TcpManager::RegisterMetaType()
{
    // 注册自定义类型
    qRegisterMetaType<RequestType>("RequestType");
    qRegisterMetaType<UserInfo>("UserInfo");
    qRegisterMetaType<FriendInfo>("FriendInfo");
    qRegisterMetaType<MessageItem>("MessageItem");
    qRegisterMetaType<ConversationItem>("ConversationItem");

    // 注册std::shared_ptr类型
    qRegisterMetaType<std::shared_ptr<UserInfo>>("std::shared_ptr<UserInfo>");
    qRegisterMetaType<std::shared_ptr<FriendInfo>>("std::shared_ptr<FriendInfo>");
    qRegisterMetaType<std::shared_ptr<MessageItem>>("std::shared_ptr<MessageItem>");
    qRegisterMetaType<std::shared_ptr<ConversationItem>>("std::shared_ptr<ConversationItem>");

    // 注册容器类型
    qRegisterMetaType<std::vector<std::shared_ptr<UserInfo>>>("std::vector<std::shared_ptr<UserInfo>>");
    qRegisterMetaType<std::vector<std::shared_ptr<MessageItem>>>("std::vector<std::shared_ptr<MessageItem>>");
    qRegisterMetaType<QList<std::shared_ptr<FriendInfo>>>("QList<std::shared_ptr<FriendInfo>>");
}

void TcpManager::do_tcp_connect(ServerInfo si)
{
    qDebug() << "Connecting to server " << si.host << ":" << si.port ;
    _host = si.host;
    _port = static_cast<uint16_t>(si.port.toInt());
    _socket.connectToHost(_host,_port);
}

void TcpManager::do_send_data(RequestType requestType, QByteArray data)
{

    if (!_socket.isOpen()){
        QMessageBox msgBox;
        msgBox.setWindowTitle("Network Issue");
        msgBox.setText("No Connection to Server");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);

               // macOS 风格样式表
        msgBox.setStyleSheet(R"(
            QMessageBox {
                background-color: #f5f5f7;
                border: 1px solid #d0d0d0;
                border-radius: 10px;
                font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            }
            QMessageBox QLabel {
                color: #1d1d1f;
                font-size: 14px;
                font-weight: 400;
                padding: 15px;
            }
            QMessageBox QLabel#qt_msgbox_label {
                min-width: 300px;
            }
            QMessageBox QPushButton {
                background-color: #007aff;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 8px 24px;
                font-size: 13px;
                font-weight: 500;
                min-width: 80px;
                margin: 5px;
            }
            QMessageBox QPushButton:hover {
                background-color: #0056d6;
            }
            QMessageBox QPushButton:pressed {
                background-color: #0040a8;
            }
            QMessageBox QPushButton:focus {
                outline: 2px solid #007aff;
                outline-offset: 2px;
            }
        )");

        msgBox.exec();

        emit on_no_connection();
        return;
    }

    auto id =static_cast<uint16_t>(requestType);

    quint16 len = static_cast<quint16>(data.size());

    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);

    out.setByteOrder(QDataStream::BigEndian);

    out << id << len;
<<<<<<< HEAD
    // 将数据追加到 block 中
    block.append(data);

    // 如果正在发送，直接入队
    if (_pending){
        _queue.enqueue(block);
=======

    // 如果正在发送，直接入队
    if (_pending){
        _queue.enqueue(data);
>>>>>>> origin/main
        return;
    }

    // 如果没有发送，那就直接发送
    _current_block = block;
    _bytes_send = 0;
    _pending = true;

    _socket.write(_current_block);
}

TcpThread::TcpThread(QWidget *parent)
{
    _thread = new QThread();
    TcpManager::GetInstance()->moveToThread(_thread);
    QObject::connect(_thread,&QThread::finished,_thread,&QObject::deleteLater);
    _thread->start();
}

TcpThread::~TcpThread()
{
    _thread->quit();
}
