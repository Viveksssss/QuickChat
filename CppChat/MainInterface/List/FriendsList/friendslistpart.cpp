#include "friendslistpart.h"
#include "frienditemdelegate.h"
#include "friendsmodel.h"
#include "frienditem.h"
#include "../../../Properties/global.h"
#include "../../../Properties/signalrouter.h"
#include "../../../tcpmanager.h"
#include "../../../usermanager.h"



#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMovie>
#include <QTimer>

FriendsListPart::FriendsListPart(QWidget *parent)
    : QWidget{parent}
    , isLoading{false}
{
    setupUI();
    setupConnections();

    do_loading_users();

}

QListView *FriendsListPart::getList()
{
    return friendsList;
}

void FriendsListPart::setupUI()
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
    title->setText("Friends");
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
    friendsList = new QListView;
    friendsList->setObjectName("friendsList");
    friendsModel = new FriendsModel(this);
    friendsDelegate = new FriendItemDelegate(this,this);
    QScrollBar *vScrollBar = friendsList->verticalScrollBar();
    vScrollBar->setSingleStep(10);  // 每次滚轮滚动10像素
    friendsList->viewport()->installEventFilter(this);
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

    friendsList->setModel(friendsModel);
    friendsList->setItemDelegate(friendsDelegate);

    friendsList->setSelectionMode(QAbstractItemView::SingleSelection);
    friendsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    friendsList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    friendsList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);   // 需要时显示
    friendsList->setMaximumWidth(250);

    // 添加到布局
    main_vlay->addLayout(top_hlay);
    main_vlay->addWidget(friendsList);
}

void FriendsListPart::setupConnections()
{
    // 滚动接受新的列表
    connect(this,&FriendsListPart::on_loading_users,this,&FriendsListPart::do_loading_users);
    // 改变登陆状态
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_change_friend_status,this,&FriendsListPart::do_change_friend_status);
    connect(TcpManager::GetInstance().get(),&TcpManager::on_change_friend_status,this,&FriendsListPart::do_change_friend_status);
    // 添加好友
    connect(TcpManager::GetInstance().get(),&TcpManager::on_add_friend_to_list,this,&FriendsListPart::do_add_friend_to_list);
    // 添加好友列表
    connect(TcpManager::GetInstance().get(),&TcpManager::on_add_friends_to_list,this,&FriendsListPart::do_add_friends_to_list);
    // 点击列表项
    connect(friendsList,&QListView::clicked,this,[this](const auto&index){
        if (!index.isValid()){
            return;
        }
        FriendItem item = friendsModel->getFriend(index.row());
        if (item.id>=0){
            emit SignalRouter::GetInstance().on_change_friend_selection(item);
        }
    });
}


bool FriendsListPart::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == friendsList->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        QScrollBar *vScrollBar = friendsList->verticalScrollBar();
        if (vScrollBar) {
            // 自定义滚动步长
            int delta = wheelEvent->angleDelta().y();
            int step = delta > 0 ? -30 : 30;  // 反向，因为滚动条值增加是向下
            vScrollBar->setValue(vScrollBar->value() + step);

            int maxValue = vScrollBar->maximum();
            int currentValue = vScrollBar->value();
            if (currentValue - maxValue >= 0 && !UserManager::GetInstance()->IsLoadFriendsFinished()){
                emit on_loading_users();
            }

            return true; // 事件已处理
        }
    }
    return QWidget::eventFilter(obj, event);
}


void FriendsListPart::do_loading_users()
{
    if(isLoading){
        return;
    }

    isLoading = true;
    // 动态获取信息
    for(auto&info:UserManager::GetInstance()->GetFriendsPerPage()){
        friendsModel->addFriend(FriendItem(info->id, info->status,info->sex,info->name,info->avatar,info->desc ));
    }

    QTimer::singleShot(1000,this,[this](){
        this->setLoading(false);
    });
}

void FriendsListPart::do_add_friend_to_list(std::shared_ptr<UserInfo>info)
{
    // 先检查是否已存在，避免重复添加
    if (friendsModel->indexFromUid(info->id).isValid()) {
        return;
    }
    friendsModel->addFriend(FriendItem(info->id, info->status,info->sex,info->name,info->avatar,info->desc ));
    UserManager::GetInstance()->AddFriendToList(info);
}

void FriendsListPart::do_add_friends_to_list(const std::vector<std::shared_ptr<UserInfo>> &list)
{
    for (auto&item:list){
        // 先检查是否已存在，避免重复添加
        if (friendsModel->indexFromUid(item->id).isValid()) {
            continue;
        }
        friendsModel->addFriend(FriendItem(item->id, item->status,item->sex,item->name,item->avatar,item->desc ));
    }
}

void FriendsListPart::do_change_friend_status(int uid,int status)
{
    auto index = friendsModel->indexFromUid(uid);
    friendsModel->setData(index,status,FriendsModel::StatusRole);
}
