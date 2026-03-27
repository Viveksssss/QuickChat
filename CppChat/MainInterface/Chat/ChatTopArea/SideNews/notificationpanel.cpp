#include "notificationpanel.h"
#include "systemnewsitem.h"
#include "friendsnewsitem.h"
#include "../../../../Properties/global.h"
#include "../../../../tcpmanager.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QListWidget>
#include <QCloseEvent>
#include <QApplication>


NotificationPanel::NotificationPanel(QWidget *parent)
    : QWidget(parent)
{
    setParent(window());
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_StyledBackground, true);

    if (parent) {
        setAttribute(Qt::WA_ShowWithoutActivating);
    }

    setupUI();
    setupConnections();
}

void NotificationPanel::addFriendNews(bool isReply,int code ,int uid, int sex,const QString &iconPath, const QString &name, const QString &content)
{
    emit on_show_red_dot();
    QString icon = (iconPath.isNull() || iconPath.isEmpty())?  ":/Resources/main/header-default.png":iconPath;
    FriendsNewsItem *itemWidget = new FriendsNewsItem(isReply,code,uid,sex,icon, name, content);
    QListWidgetItem*item = new QListWidgetItem;
    item->setSizeHint(itemWidget->sizeHint());

    friendsNews->addItem(item);
    friendsNews->setItemWidget(item,itemWidget);

    friendsNews->update();

    connect(itemWidget, &FriendsNewsItem::on_accepted_clicked, [this, item]() {
        do_friend_accept(item);
    });
    connect(itemWidget, &FriendsNewsItem::on_rejected_clicked, [this, item]() {
        do_friend_reject(item);
    });
    connect(itemWidget, &FriendsNewsItem::on_confirm_clicked, [this, item]() {
        do_friend_confirm_clicked(item);
    });

}

void NotificationPanel::addSystemNews(bool isReply, int uid, const QString &iconPath, const QString &name, const QString &content)
{
    emit on_show_red_dot();
    SystemNewsItem *itemWidget = new SystemNewsItem(isReply,uid,iconPath, name, content);
    QListWidgetItem*item = new QListWidgetItem;
    item->setSizeHint(itemWidget->sizeHint());

    systemNews->addItem(item);
    systemNews->setItemWidget(item,itemWidget);

    systemNews->update();

    connect(itemWidget, &SystemNewsItem::on_accepted_clicked, [this, item]() {
        do_system_accept(item);
    });
    connect(itemWidget, &SystemNewsItem::on_rejected_clicked, [this, item]() {
        do_system_reject(item);
    });
    connect(itemWidget, &SystemNewsItem::on_confirm_clicked, [this, item]() {
        do_system_confirm_clicked(item);
    });
}


void NotificationPanel::showPanel()
{
    if (parentWidget() != nullptr){
        QWidget*mainWindow = parentWidget();
        QRect parentR = mainWindow->geometry();

        QRect startRect = QRect(parentR.width(),10,width(),mainWindow->height() - 20);
        QRect endRect(parentR.width() - width() - 10,10,width(),mainWindow->height() - 20);
        setGeometry(startRect);

        friendsNews->setFixedHeight(height()/2-80);

        show();
        raise();
        slideAnimation->setStartValue(startRect);
        slideAnimation->setEndValue(endRect);
        slideAnimation->start();
    }
}

void NotificationPanel::hidePanel()
{
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    fadeAnimation->start();
    connect(fadeAnimation, &QPropertyAnimation::finished, this, [this](){
        hide();
    });
}

void NotificationPanel::setupUI()
{
    QVBoxLayout *main_vlay = new QVBoxLayout(this);
    main_vlay->setContentsMargins(0,0,0,0);
    main_vlay->setSpacing(10);

    QLabel *title = new QLabel;
    title->setText("通知");
    QPalette palette = title->palette();
    palette.setColor(QPalette::WindowText,Qt::black);
    title->setPalette(palette);
    QFont font = title->font();
    font.setPointSize(16);
    font.setBold(true);
    title->setFont(font);
    title->setFixedHeight(40);
    title->setAlignment(Qt::AlignCenter);

    // 好友通知标签
    QLabel *friendLabel = new QLabel("好友请求");
    friendLabel->setFixedHeight(30);

    // 好友通知列表
    friendsNews = new QListWidget;
    friendsNews->setSpacing(1);

    // 系统通知标签
    QLabel *systemLabel = new QLabel("系统通知");
    systemLabel->setFixedHeight(30);

    // 系统通知列表
    systemNews = new QListWidget;
    systemNews->setSpacing(1);

    main_vlay->addWidget(title);
    main_vlay->addWidget(friendLabel);
    main_vlay->addWidget(friendsNews);
    main_vlay->addWidget(systemLabel);
    main_vlay->addWidget(systemNews);

    // 创建动画
    slideAnimation = new QPropertyAnimation(this,"geometry",this);
    slideAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    slideAnimation->setDuration(100);

    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(1.0);
    setGraphicsEffect(opacityEffect);   // 接管绘制

    fadeAnimation = new QPropertyAnimation(this);
    fadeAnimation->setEasingCurve(QEasingCurve::Linear);
    fadeAnimation->setTargetObject(opacityEffect);
    fadeAnimation->setDuration(100);

}

void NotificationPanel::setupConnections()
{
    connect(TcpManager::GetInstance().get(),&TcpManager::on_get_apply_list,this,&NotificationPanel::do_get_apply_list);
    connect(TcpManager::GetInstance().get(),&TcpManager::on_auth_friend,this,&NotificationPanel::do_auth_friend);
    connect(TcpManager::GetInstance().get(),&TcpManager::on_add_friend,this,&NotificationPanel::do_add_friend);
    connect(TcpManager::GetInstance().get(),&TcpManager::on_notify_friend,this,&NotificationPanel::do_notify_friend);
    connect(TcpManager::GetInstance().get(),&TcpManager::on_notify_friend2,this,&NotificationPanel::do_notify_friend2);
    connect(TcpManager::GetInstance().get(),&TcpManager::on_notifications_to_list,this,&NotificationPanel::do_notifications_to_list);
}

void NotificationPanel::checkIsEmpty()
{
    if (!friendsNews->count()&&!systemNews->count()){
        emit on_unshow_red_dot();
    }
}

void NotificationPanel::do_friend_accept(QListWidgetItem *item)
{
    int row = friendsNews->row(item);
    friendsNews->takeItem(row);
    delete item;
    checkIsEmpty();
}

void NotificationPanel::do_friend_reject(QListWidgetItem *item)
{
    int row = friendsNews->row(item);
    friendsNews->takeItem(row);
    delete item;
    checkIsEmpty();
}

void NotificationPanel::do_friend_confirm_clicked(QListWidgetItem *item)
{
    int row = friendsNews->row(item);
    friendsNews->takeItem(row);
    delete item;
    checkIsEmpty();
}

void NotificationPanel::do_system_accept(QListWidgetItem *item)
{
    int row = systemNews->row(item);
    systemNews->takeItem(row);
    delete item;
    checkIsEmpty();
}

void NotificationPanel::do_system_reject(QListWidgetItem *item)
{
    int row = systemNews->row(item);
    systemNews->takeItem(row);
    delete item;
    checkIsEmpty();
}

void NotificationPanel::do_system_confirm_clicked(QListWidgetItem *item)
{
    int row = systemNews->row(item);
    systemNews->takeItem(row);
    delete item;
    checkIsEmpty();
}

void NotificationPanel::do_auth_friend(std::shared_ptr<UserInfo> info)
{
    addFriendNews(false,info->status,info->id,info->sex,info->avatar,info->name,"😄向您发来好友申请😄");
}

void NotificationPanel::do_notifications_to_list(const std::vector<std::shared_ptr<UserInfo>> &list)
{
    for(auto&item:list){
        addFriendNews(true,item->status,item->id,-1,item->avatar,"时间"+item->back,item->desc);
    }
}

void NotificationPanel::do_notify_friend(std::shared_ptr<UserInfo> info, bool accept)
{
    if (accept)
        addFriendNews(true,info->status,info->id,info->sex,info->avatar,info->name,"😄双方已建立好友关系！😄");
}

void NotificationPanel::do_notify_friend2(std::shared_ptr<UserInfo> info, bool accept)
{
    if (accept)
        addFriendNews(true,info->status,info->id,info->sex,info->avatar,info->name,"😄对方同意了您的好友申请😄");
    else
        addFriendNews(true,info->status,info->id,info->sex,info->avatar,info->name,"😢对方拒绝了您的好友请求😢");
}

void NotificationPanel::do_get_apply_list(const std::vector<std::shared_ptr<UserInfo>>&list)
{

    for(const auto&apply:list){
        addFriendNews(false,apply->status,apply->id,apply->sex,apply->avatar,apply->name,"😄向您发来好友申请😄");
    }
}

void NotificationPanel::do_add_friend(const UserInfo &info)
{
    /*addFriendNews(bool isReply, int uid, int sex,
     * const QString &iconPath, const QString &name, const QString &content) */
    addFriendNews(true,info.status,info.id,-1,":/Resources/main/header-default.png","好友申请通知",QString("😄向用户 %1 的请求已发出😄").arg(info.id));
}


void NotificationPanel::closeEvent(QCloseEvent *event)
{
    hidePanel();
    event->ignore();
}

void NotificationPanel::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}
