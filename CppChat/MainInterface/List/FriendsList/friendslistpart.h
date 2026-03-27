#ifndef FRIENDSLISTPART_H
#define FRIENDSLISTPART_H


/******************************************************************************
 *
 * @file       friendslistpart.h
 * @brief      好友列表
 *
 * @author     Vivek
 * @date       2025/10/30
 * @history
 *****************************************************************************/

#include <QWidget>

class FriendsModel;
class FriendItemDelegate;
class QListView;
class QPushButton;
class QLabel;
class MessageItem;
struct UserInfo;
class FriendsListPart : public QWidget
{
    Q_OBJECT
    friend class FriendItemDelegate;
public:
    explicit FriendsListPart(QWidget *parent = nullptr);
    QListView* getList();
private:
    void setupUI();
    void setupConnections();
    std::shared_ptr<UserInfo> userFor(const QModelIndex&index);
signals:
    void on_loading_users(); // to do_loading_users

private slots:
    void do_loading_users();
    void do_add_friend_to_list(std::shared_ptr<UserInfo>); // from TcpManager::on_add_frined_to_list;
    void do_add_friends_to_list(const std::vector<std::shared_ptr<UserInfo>>&list); // from TcpManager::on_add_frineds_to_list;
    void do_change_friend_status(int,int);  // from FriendsNewsItem->SignalRouter::on_change_friend_status;

private:
    QLabel *title;
    QPushButton *findButton;
    QListView *friendsList;
    FriendsModel *friendsModel;
    FriendItemDelegate *friendsDelegate;
    // 是否正在加载列表
    bool isLoading;


    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
    inline bool getIsLoading()noexcept{return isLoading;}
    inline void setLoading(bool loading)noexcept{this->isLoading = loading;}
};

#endif // FRIENDSLISTPART_H
