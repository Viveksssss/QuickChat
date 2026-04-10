#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include "Properties/singleton.h"
#include <QObject>
#include <memory>
#include <QTcpSocket>
#include <QHash>
#include <functional>
#include <QThread>
#include <QQueue>

class QTimer;
struct ConversationItem;
struct MessageItem;
class TcpManager
    : public QObject
    , public Singleton<TcpManager>
    , public std::enable_shared_from_this<TcpManager>
{
    Q_OBJECT
    friend class Singleton<TcpManager>;
public:
    ~TcpManager();
    bool isConnected();
    // 返回是否真正登陆
    inline bool isOk()noexcept{return _ok;}

private:
    TcpManager();
    // 注册回调
    void initHandlers();
    // 连接
    void connections();
    // 处理单个数据->hash找回调处理
    void handleMessage(RequestType requestType,int len,QByteArray data);
    // 注册元对象
    void RegisterMetaType();

private:
    // 连接实例
    QTcpSocket _socket;
    // 地址
    QString _host;
    // 端口
    uint16_t _port;
    // 缓存容器
    QByteArray _buffer;
    // 处理包头
    bool _recv_pending;
    // Type
    quint16 _msg_id;
    // Length
    quint16 _msg_len;
    // 存放请求和对应的回调函数
    QHash<RequestType,std::function<void(RequestType,int,QByteArray)>>_handlers;
    // 是否真正连接登陆
    bool _ok;
    // 发送队列
    QQueue<QByteArray>_queue;
    // 一发送长度
    quint64 _bytes_send;
    // 是否正在发送
    bool _pending;
    // 正在发送的包
    QByteArray _current_block;

public slots:
    void do_tcp_connect(ServerInfo); // from LoginScreen::on_tcp_connect
    void do_send_data(RequestType requestType,QByteArray data); // from TcpManager::to_send_data
signals:
    void on_switch_login();         //  to MainWindow::
    void on_connect_success(bool success); // to LoginScreen::do_connect_success
    void on_send_data(RequestType requestType,QByteArray data); // to TcpManager::do_send_data
    void on_switch_interface(); // to MainWindow::[](){}
    void on_login_failed(int);  // to LoginScreen::do_login_failed
    void on_users_searched(QList<std::shared_ptr<FriendInfo>>list);   // to AnimatedSearchBox::do_users_searched
    void on_add_friend(const UserInfo&info); // to NotifycationPanel::do_add_friend
    void on_auth_friend(std::shared_ptr<UserInfo>info); // to NotificationPanel::do_auth_friend ; TopChatArea::do_show_red_dot
    void on_get_apply_list(const std::vector<std::shared_ptr<UserInfo>>&list); // to NotificationPanel::do_get_apply_list;
    void on_add_friend_to_list(std::shared_ptr<UserInfo>); // to FriendListPart::do_add_friend_to_list
    void on_add_friends_to_list(std::vector<std::shared_ptr<UserInfo>>list); // to FriendListPart::do_add_friends_to_list
    void on_add_messages_to_list(const std::vector<std::shared_ptr<ConversationItem>>&list); // to MessageListPart::do_add_messages_to_list
    void on_notifications_to_list(const std::vector<std::shared_ptr<UserInfo>>&list);// to NotificationPanel::do_notifications_to_list
    void on_notify_friend(std::shared_ptr<UserInfo>info,bool accept);   // to NotificationPanel::do_notify_friend
    void on_notify_friend2(std::shared_ptr<UserInfo>info,bool accept);   // to NotificationPanel::do_notify_friend2
    void on_change_friend_status(int,int);  // to FriendsListPart::do_change_friend_status;
    void on_change_chat_history(std::vector<std::shared_ptr<MessageItem>>); // to ChatArea::do_change_chat_history;
    void on_get_message(const MessageItem&);// to MessageListPart::do_get_message
    void on_get_messages(const std::vector<std::shared_ptr<MessageItem>>&lists);    // to MessageListPart::do_get_messages
    void on_connection_closed();
    void on_no_connection();
};

class TcpThread:public std::enable_shared_from_this<TcpThread>
{
public:
    explicit TcpThread(QWidget*parent=nullptr);
    ~TcpThread();
private:
    QThread *_thread;
};

#endif // TCPMANAGER_H
