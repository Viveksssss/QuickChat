// #include "messageslistpart.h"
// #include "messageitemdelegate.h"
// #include "messagesmodel.h"
// #include "../../../../Properties/global.h"
// #include "../../../../Properties/signalrouter.h"
// #include "../../../../tcpmanager.h"
// #include "../../../../usermanager.h"
// #include "../../../../database.h"



// #include <QHBoxLayout>
// #include <QLabel>
// #include <QListView>
// #include <QPushButton>
// #include <QAbstractItemView>
// #include <QScrollBar>
// #include <QWheelEvent>
// #include <QMovie>
// #include <QTimer>


// MessagesListPart::MessagesListPart(QWidget *parent)
//     : QWidget{parent}
//     , isLoading{false}
// {
//     setupUI();
//     setupConnections();

//     // do_loading_messages();
//     QTimer::singleShot(100, this, [this]() {
//         if (!UserManager::GetInstance()->GetMessages().empty()) {
//             do_loading_messages();
//         } else {
//             qDebug() << "No messages available, waiting for network data...";
//         }
//     });

// }

// QListView *MessagesListPart::getList()
// {
//     return messagesList;
// }

// void MessagesListPart::setupUI()
// {
//     setMinimumWidth(40);
//     setMaximumWidth(220);

//     setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

//     QVBoxLayout *main_vlay = new QVBoxLayout(this);
//     main_vlay->setContentsMargins(3,0,8,15);

//     QHBoxLayout *top_hlay = new QHBoxLayout;
//     top_hlay->setContentsMargins(15,0,15,0);

//     // title
//     title = new QLabel;
//     title->setText("Messages");
//     title->setAlignment(Qt::AlignVCenter);

//     auto font = title->font();
//     font.setWeight(QFont::Black);
//     title->setFont(font);
//     top_hlay->addWidget(title);
//     top_hlay->addStretch();

//     // button
//     findButton = new QPushButton;
//     findButton->setFixedSize({30,30});
//     findButton->setObjectName("findButton");
//     findButton->setIcon(QIcon(":/Resources/main/find.png"));
//     findButton->setIconSize(QSize(20,20));
//     top_hlay->addWidget(findButton,Qt::AlignVCenter);

//     //listView
//     messagesList = new QListView;
//     messagesList->setObjectName("messagesList");
//     messagesModel = new MessagesModel(this);
//     messagesDelegate = new MessageItemDelegate(this,this);
//     QScrollBar *vScrollBar = messagesList->verticalScrollBar();
//     vScrollBar->setSingleStep(10);  // 每次滚轮滚动10像素
//     messagesList->viewport()->installEventFilter(this);
//     vScrollBar->setStyleSheet(
//         "QScrollBar{"
//         "background:transparent;"
//         "width:3px;"
//         "border-radius:3px;"
//         "}"
//         "QScrollBar::handle:vertical {"
//         "background:#eae6e9;"
//         "border-radius:10px;"
//         "margin-left:2px;"
//         "}"
//         "QScrollBar::handle:vertical:hover{"
//         "background:#efa3e2;"
//         "border-radius:10px;"
//         "}"
//         "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
//         "border: none;"
//         "background: none;"
//         "height: 0px;"
//         "}"
//     );

//     messagesList->setModel(messagesModel);
//     messagesList->setItemDelegate(messagesDelegate);

//     messagesList->setSelectionMode(QAbstractItemView::SingleSelection);
//     messagesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//     messagesList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
//     messagesList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);   // 需要时显示
//     messagesList->setMaximumWidth(250);

//     // 添加到布局
//     main_vlay->addLayout(top_hlay);
//     main_vlay->addWidget(messagesList);
// }

// void MessagesListPart::setupConnections()
// {
//     // 滚动接受新的列表
//     connect(this,&MessagesListPart::on_loading_messages,this,&MessagesListPart::do_loading_messages);
//     // 添加消息
//     connect(&SignalRouter::GetInstance(),&SignalRouter::on_add_message_to_list,this,&MessagesListPart::do_add_message_to_list);
//     // 添加消息列表
//     connect(TcpManager::GetInstance().get(),&TcpManager::on_add_messages_to_list,this,&MessagesListPart::do_add_messages_to_list);
//     // 点击列表项
//     connect(messagesList,&QListView::clicked,this,[this](const auto&index){
//         if (!index.isValid()){
//             return;
//         }
//         ConversationItem item = messagesModel->getMessage(index.row());
//         if (item.to_uid>=0){
//             emit SignalRouter::GetInstance().on_change_message_selection(item);
//         }
//     });
//     // 切换列表
//     connect(&SignalRouter::GetInstance(),&SignalRouter::on_message_item,this,&MessagesListPart::do_change_peer);
//     // 有新消息来临
//     connect(TcpManager::GetInstance().get(),&TcpManager::on_get_message,this,&MessagesListPart::do_get_message);
//     connect(TcpManager::GetInstance().get(), &TcpManager::on_add_messages_to_list,
//             this, &MessagesListPart::do_data_ready);
// }


// void MessagesListPart::do_data_ready(const std::span<std::shared_ptr<ConversationItem>> &list)
// {
//     qDebug() << "Messages data ready, count:" << list.size();
//     dataReady = true;

//     // 数据到达后，执行初始加载
//     do_loading_messages();

//     // 同时调用原有的处理函数
//     do_add_messages_to_list(list);
// }


// bool MessagesListPart::eventFilter(QObject *obj, QEvent *event)
// {
//     if (obj == messagesList->viewport() && event->type() == QEvent::Wheel) {
//         QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
//         QScrollBar *vScrollBar = messagesList->verticalScrollBar();
//         if (vScrollBar) {
//             // 自定义滚动步长
//             int delta = wheelEvent->angleDelta().y();
//             int step = delta > 0 ? -30 : 30;  // 反向，因为滚动条值增加是向下
//             vScrollBar->setValue(vScrollBar->value() + step);

//             int maxValue = vScrollBar->maximum();
//             int currentValue = vScrollBar->value();
//             UserManager::GetInstance();
//             if (currentValue - maxValue >= 0 && !UserManager::GetInstance()->IsLoadMessagesFinished()){
//                 qDebug() << "load more messages";
//                 emit on_loading_messages();
//             }

//             return true; // 事件已处理
//         }
//     }
//     return QWidget::eventFilter(obj, event);
// }


// void MessagesListPart::do_loading_messages()
// {
//     if(isLoading){
//         return;
//     }
//     isLoading = true;

//     // 动态获取信息
//     for(auto&info:UserManager::GetInstance()->GetMessagesPerPage()){
//         messagesModel->addMessage(*info);
//     }

//     QTimer::singleShot(1000,this,[this](){
//         this->setLoading(false);
//     });
// }

// void MessagesListPart::do_add_message_to_list(const ConversationItem &info)
// {
//     messagesModel->addMessage(info);
//     messagesList->update();
//     messagesList->viewport()->update();
//     do_loading_messages();
// }

// void MessagesListPart::do_add_messages_to_list(const std::span<std::shared_ptr<ConversationItem> > &list)
// {
//     for (auto&item:list){
//         messagesModel->addMessage(*item);
//     }
//     messagesList->update();
//     messagesList->viewport()->update();
//     do_loading_messages();
// }

// void MessagesListPart::do_change_message_status(int uid,int status)
// {
//     auto index = messagesModel->indexFromUid(uid);
//     messagesModel->setData(index,status,MessagesModel::StatusRole);
// }

// void MessagesListPart::do_change_peer(int peerUid)
// {
//     auto index = messagesModel->indexFromUid(peerUid);
//     if (index.isValid()){
//         messagesList->setCurrentIndex(index);   // 切换列表索引至当前好友
//         DataBase::GetInstance().updateMessagesStatus(peerUid,1);
//     }else{
//         ConversationItem conv;
//         conv.to_uid = peerUid;
//         conv.from_uid = UserManager::GetInstance()->GetUid();
//         conv.name = UserManager::GetInstance()->GetPeerName();
//         conv.icon = UserManager::GetInstance()->GetPeerIcon();
//         messagesModel->addPreMessage(conv);
//         messagesList->setCurrentIndex(messagesModel->index(0, 0));
//     }
// }

// void MessagesListPart::do_get_message(const MessageItem &message)
// {
//     int peerUid = message.from_id;
//     // 先直接存储到数据库
//     if (messagesModel->existMessage(peerUid)){
//         // 更新最近消息
//         messagesModel->setData(messagesModel->indexFromUid(peerUid),message.content.data,MessagesModel::MessageRole);
//         if (messagesModel->indexFromUid(peerUid) == messagesList->currentIndex()){
//             // 添加message
//             emit SignalRouter::GetInstance().on_add_new_message(message);
//             auto p = std::move(message);
//             p.status = 1;
//             DataBase::GetInstance().storeMessage(p);
//         }else{
//             // 红点
//             do_change_message_status(peerUid,false);
//             DataBase::GetInstance().storeMessage(message);
//         }
//     }else{
//         DataBase::GetInstance().storeMessage(message);
//         // 不存在会话，那就创建插入会话
//         std::shared_ptr<UserInfo>info = DataBase::GetInstance().getFriendInfoPtr(peerUid);
//         ConversationItem conv;
//         // 这里故意反过来，对于自己来说所有的好友都是to,自己是from
//         conv.to_uid = message.from_id;
//         conv.from_uid = message.to_id;
//         conv.icon = info->avatar;
//         if (message.content.type == MessageType::TextMessage){
//             conv.message = message.content.data.toString();
//         }else{
//             //TODO:比如图片，文件。。。
//             conv.message = "Other:暂时没做";
//         }
//         conv.pined = 0;
//         conv.status = 1;
//         conv.create_time = QDateTime::currentDateTime();
//         conv.update_time = QDateTime::currentDateTime();
//         conv.deleted = 0;
//         conv.name = info->name;
//         conv.processed = false;
//         messagesModel->addPreMessage(conv);
//         UserManager::GetInstance()->setHistoryTimestamp(conv.from_uid,QDateTime::currentDateTime());
//         // 存放数据库中
//         DataBase::GetInstance().createOrUpdateConversation(conv);
//         // TODO:发给服务器
//     }
// }

// void MessagesListPart::do_change_message_status(int peerUid, bool processed)
// {
//     messagesModel->setData(messagesModel->indexFromUid(peerUid),processed,MessagesModel::MessageRole::RedDotRole);
// }


#include "messageslistpart.h"
#include "messageitemdelegate.h"
#include "messagesmodel.h"
#include "../../../../Properties/global.h"
#include "../../../../Properties/signalrouter.h"
#include "../../../../tcpmanager.h"
#include "../../../../usermanager.h"
#include "../../../../database.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMovie>
#include <QTimer>
#include <QJsonArray>

MessagesListPart::MessagesListPart(QWidget *parent)
    : QWidget{parent}
    , isLoading{false}
    , dataReady(false)  // 添加数据就绪标志
{
    setupUI();
    setupConnections();

    // 延迟检查数据，避免立即加载时数据还未准备好
    // QTimer::singleShot(200, this, [this]() {
        if (!UserManager::GetInstance()->GetMessages().empty()) {
            do_loading_messages();
        } else {
            // 如果没有数据，设置一个重试机制
            QTimer::singleShot(1000, this, [this]() {
                if (!dataReady && !UserManager::GetInstance()->GetMessages().empty()) {
                    do_loading_messages();
                }
            });
        }
    // });
}

MessagesListPart::~MessagesListPart()
{
    syncConversations();
}

QListView *MessagesListPart::getList()
{
    return messagesList;
}

void MessagesListPart::setupUI()
{
    setMinimumWidth(40);
    setMaximumWidth(220);

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QVBoxLayout *main_vlay = new QVBoxLayout(this);
    main_vlay->setContentsMargins(3,0,8,15);

    QHBoxLayout *top_hlay = new QHBoxLayout;
    top_hlay->setContentsMargins(15,0,15,0);

    // title
    title = new QLabel;
    title->setText("Messages");
    title->setAlignment(Qt::AlignVCenter);
    title->setStyleSheet("color:black");

    auto font = title->font();
    font.setWeight(QFont::Black);
    title->setFont(font);
    top_hlay->addWidget(title);
    top_hlay->addStretch();

    // button
    findButton = new QPushButton;
    findButton->setFixedSize({30,30});
    findButton->setObjectName("findButton");
    findButton->setIcon(QIcon(":/Resources/main/find.png"));
    findButton->setIconSize(QSize(20,20));
    top_hlay->addWidget(findButton,Qt::AlignVCenter);

    //listView
    messagesList = new QListView;
    messagesList->setObjectName("messagesList");
    messagesModel = new MessagesModel(this);
    messagesDelegate = new MessageItemDelegate(this,this);
    QScrollBar *vScrollBar = messagesList->verticalScrollBar();
    vScrollBar->setSingleStep(10);  // 每次滚轮滚动10像素
    messagesList->viewport()->installEventFilter(this);
    vScrollBar->setStyleSheet(
        "QScrollBar{"
        "background:transparent;"
        "width:3px;"
        "border-radius:3px;"
        "}"
        "QScrollBar::handle:vertical {"
        "background:#eae6e9;"
        "border-radius:10px;"
        "margin-left:2px;"
        "}"
        "QScrollBar::handle:vertical:hover{"
        "background:#efa3e2;"
        "border-radius:10px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "border: none;"
        "background: none;"
        "height: 0px;"
        "}"
        );

    messagesList->setModel(messagesModel);
    messagesList->setItemDelegate(messagesDelegate);

    messagesList->setSelectionMode(QAbstractItemView::SingleSelection);
    messagesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messagesList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    messagesList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);   // 需要时显示
    messagesList->setMaximumWidth(250);

    // 添加到布局
    main_vlay->addLayout(top_hlay);
    main_vlay->addWidget(messagesList);

    _wait_sync_conversations.reserve(15);
}

void MessagesListPart::setupConnections()
{
    // 滚动接受新的列表
    connect(this,&MessagesListPart::on_loading_messages,this,&MessagesListPart::do_loading_messages);
    // 添加消息
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_add_message_to_list,this,&MessagesListPart::do_add_message_to_list);
    // 添加消息列表
    connect(TcpManager::GetInstance().get(),&TcpManager::on_add_messages_to_list,this,&MessagesListPart::do_add_messages_to_list);
    // 点击列表项
    connect(messagesList,&QListView::clicked,this,[this](const auto&index){
        if (!index.isValid()){
            return;
        }
        ConversationItem item = messagesModel->getMessage(index.row());
        if (item.to_uid>=0){
            emit SignalRouter::GetInstance().on_change_message_selection(item);
        }
    });
    // 切换列表
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_change_peer,this,&MessagesListPart::do_change_peer);
    // 有新消息来临
    connect(TcpManager::GetInstance().get(),&TcpManager::on_get_message,this,&MessagesListPart::do_get_message);
    connect(TcpManager::GetInstance().get(),&TcpManager::on_get_messages,this,&MessagesListPart::do_get_messages);
    // 数据就绪信号
    connect(TcpManager::GetInstance().get(), &TcpManager::on_add_messages_to_list,
            this, &MessagesListPart::do_data_ready);
}

void MessagesListPart::do_data_ready(const std::vector<std::shared_ptr<ConversationItem>> &list)
{
    dataReady = true;

    // 数据到达后，执行初始加载
    do_loading_messages();

    // 同时调用原有的处理函数
    do_add_messages_to_list(list);

    // 确保视图完全刷新
    messagesList->update();
    messagesList->viewport()->update();
    messagesList->repaint();
}

bool MessagesListPart::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == messagesList->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        QScrollBar *vScrollBar = messagesList->verticalScrollBar();
        if (vScrollBar) {
            // 自定义滚动步长
            int delta = wheelEvent->angleDelta().y();
            int step = delta > 0 ? -30 : 30;  // 反向，因为滚动条值增加是向下
            vScrollBar->setValue(vScrollBar->value() + step);

            int maxValue = vScrollBar->maximum();
            int currentValue = vScrollBar->value();
            if (currentValue - maxValue >= 0 && !UserManager::GetInstance()->IsLoadMessagesFinished()){
                emit on_loading_messages();
            }

            return true; // 事件已处理
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MessagesListPart::do_loading_messages()
{
    if(isLoading){
        return;
    }

    // 检查是否有数据可加载
    auto messages = UserManager::GetInstance()->GetMessagesPerPage();
    if (messages.empty()) {
        // 如果没有数据，延迟重试
        QTimer::singleShot(500, this, &MessagesListPart::do_loading_messages);
        return;
    }

    isLoading = true;

    // 清空现有数据，确保从正确位置开始加载
    messagesModel->getList().clear();

    // 动态获取信息
    for(auto& info : messages) {
        messagesModel->addMessage(*info);
    }

    // 强制视图刷新
    messagesList->update();
    messagesList->viewport()->update();
    messagesList->scrollToTop();  // 滚动到顶部确保内容可见

    QTimer::singleShot(1000, this, [this](){
        this->setLoading(false);
    });
}

void MessagesListPart::do_add_message_to_list(const ConversationItem &info)
{
    messagesModel->addMessage(info);

    // 强制视图刷新
    messagesList->update();
    messagesList->viewport()->update();

    // 不需要调用 do_loading_messages()，因为这是单个添加
}

void MessagesListPart::do_add_messages_to_list(const std::vector<std::shared_ptr<ConversationItem>> &list)
{
    if (list.empty()) {
        return;
    }

    // 清空现有数据
    messagesModel->getList().clear();

    for (auto& item : list) {
        messagesModel->addMessage(*item);
    }

    // 强制视图刷新
    messagesList->update();
    messagesList->viewport()->update();
    messagesList->scrollToTop();

    // 标记数据已就绪
    dataReady = true;

}

void MessagesListPart::do_change_message_status(int uid,int status)
{
    auto index = messagesModel->indexFromUid(uid);
    if (index.isValid()) {
        messagesModel->setData(index, status, MessagesModel::StatusRole);
    }
}

void MessagesListPart::do_change_peer(int peerUid)
{
    auto index = messagesModel->indexFromUid(peerUid);
    if (index.isValid()){
        messagesModel->setData(index,true,MessagesModel::MessageRole::RedDotRole);
        messagesList->setCurrentIndex(index);   // 切换列表索引至当前好友
        DataBase::GetInstance().updateMessagesStatus(peerUid,1);
        // emit SignalRouter::GetInstance().on_change_peer(peerUid);
    }else{
        ConversationItem conv;
        conv.to_uid = peerUid;
        conv.from_uid = UserManager::GetInstance()->GetUid();
        qDebug() << UserManager::GetInstance()->GetPeerName();
        conv.name = UserManager::GetInstance()->GetPeerName();
        conv.icon = UserManager::GetInstance()->GetPeerIcon();
        conv.env = static_cast<int>(UserManager::GetInstance()->GetEnv());
        conv.status = 1;
        conv.create_time = QDateTime::currentDateTime();
        conv.update_time = QDateTime::currentDateTime();
        conv.processed = true;
        conv.deleted = 0;
        conv.pined = 0;
        messagesModel->addPreMessage(conv);
        _wait_sync_conversations.push_back(conv);
        messagesList->setCurrentIndex(messagesModel->index(0, 0));
        DataBase::GetInstance().createOrUpdateConversation(conv);
        UserManager::GetInstance()->GetMessages().push_back(std::make_shared<ConversationItem>(std::move(conv)));
        UserManager::GetInstance()->ResetLoadMessages();
        if (_wait_sync_conversations.size() >= 10){
            syncConversations();
        }
    }
    messagesList->update();
}

void MessagesListPart::do_get_messages(const std::vector<std::shared_ptr<MessageItem> > &list)
{

    for (auto &message:list){
        do_get_message(*message);
    }

    // 刷新视图
    // messagesList->update();
    // messagesList->viewport()->update();
}

void MessagesListPart::do_get_message(const MessageItem &message)
{
    int peerUid = message.from_id;

    // 先直接存储到数据库
    if (messagesModel->existMessage(peerUid)){
        // 更新最近消息
        messagesModel->setData(messagesModel->indexFromUid(peerUid), message.content.data, MessagesModel::MessageRole);
        if (messagesModel->indexFromUid(peerUid) == messagesList->currentIndex()){
            // 添加message
            emit SignalRouter::GetInstance().on_add_new_message(message);
            auto p = message;
            p.status = 1;
            DataBase::GetInstance().storeMessage(p);
        }else{
            // 红点
            do_change_message_status(peerUid, false);
            DataBase::GetInstance().storeMessage(message);
        }
    }else{
        DataBase::GetInstance().storeMessage(message);
        // 不存在会话，那就创建插入会话
        std::shared_ptr<UserInfo> info = DataBase::GetInstance().getFriendInfoPtr(peerUid);
        ConversationItem conv;
        // 这里故意反过来，对于自己来说所有的好友都是to,自己是from
        conv.to_uid = message.from_id;
        conv.from_uid = message.to_id;
        conv.icon = info ? info->avatar : "";
        if (message.content.type == MessageType::TextMessage){
            conv.message = message.content.data.toString();
        }else{
            //TODO:比如图片，文件。。。
            conv.message = "Other:暂时没做";
        }
        conv.pined = 0;
        conv.status = 1;
        conv.create_time = QDateTime::currentDateTime();
        conv.update_time = QDateTime::currentDateTime();
        conv.deleted = 0;
        conv.name = info->name;
        conv.processed = false;
        conv.env = static_cast<int>(UserManager::GetInstance()->GetEnv());
        messagesModel->addPreMessage(conv);
        messagesList->update();
        // 存放数据库中
        DataBase::GetInstance().createOrUpdateConversation(conv);
        UserManager::GetInstance()->GetMessages().push_back(std::make_shared<ConversationItem>(std::move(conv)));

        _wait_sync_conversations.push_back(std::move(conv));
        if (_wait_sync_conversations.size() >= 10){
            syncConversations();
        }
    }

    // 刷新视图
    messagesList->update();
    messagesList->viewport()->update();
}

void MessagesListPart::do_change_message_status(int peerUid, bool processed)
{
    auto index = messagesModel->indexFromUid(peerUid);
    if (index.isValid()) {
        messagesModel->setData(index, processed, MessagesModel::RedDotRole);
    }
}

// 添加设置加载状态的方法
void MessagesListPart::setLoading(bool loading)
{
    isLoading = loading;
}

void MessagesListPart::syncConversations()
{
    if (!_wait_sync_conversations.empty()){
        QJsonArray array;
        for (const auto&item:_wait_sync_conversations){
            QJsonObject val;
            val["uid"] = item.id;
            val["from_uid"] = item.from_uid;
            val["to_uid"] = item.to_uid;
            val["create_time"] = item.create_time.toString("yyyy-MM-dd HH:mm:ss");
            val["update_time"] = item.update_time.toString("yyyy-MM-dd HH:mm:ss");
            val["name"] = item.name;
            val["icon"] = item.icon;
            val["status"] = item.status;
            val["deleted"] = item.deleted;
            val["pined"] = item.deleted;
            val["processed"] = item.processed;
            array.append(val);
        }
        QJsonObject obj;
        obj["conversations"] = array;
        obj["uid"] = UserManager::GetInstance()->GetUid();
        QJsonDocument doc(obj);
        TcpManager::GetInstance()->do_send_data(RequestType::ID_SYNC_CONVERSATIONS_REQ,doc.toJson(QJsonDocument::Compact));
        _wait_sync_conversations.clear();
    }
}
