#ifndef NOTIFICATIONPANEL_H
#define NOTIFICATIONPANEL_H

#include <QWidget>
#include <QObject>
#include <QGraphicsOpacityEffect>

class QListWidgetItem;
class QPropertyAnimation;
class QListWidget;
class FriendsNewsItem;
class SystemNewsItem;
enum class NotificationCodes;
struct UserInfo;
class NotificationPanel:public QWidget
{
    Q_OBJECT
public:
    explicit NotificationPanel(QWidget*parent = nullptr);
    ~NotificationPanel() = default;
    void addFriendNews(bool isReply,int code,int uid,int sex,const QString &iconPath, const QString &name, const QString &content);
    void addSystemNews(bool isReply,int uid,const QString &iconPath, const QString &name, const QString &content);
    void showPanel();
    void hidePanel();
    void setupUI();
    void setupConnections();
    void checkIsEmpty();
signals:
    void on_unshow_red_dot();  // to TopChatArea::do_unshow_red_news();
    void on_show_red_dot(); // to TopChatArea::do_show_red_news();
public slots:
    void do_friend_accept(QListWidgetItem*item);
    void do_friend_reject(QListWidgetItem*item);
    void do_friend_confirm_clicked(QListWidgetItem*item);

    void do_system_accept(QListWidgetItem*item);
    void do_system_reject(QListWidgetItem*item);
    void do_system_confirm_clicked(QListWidgetItem*item);

    void do_get_apply_list(const std::vector<std::shared_ptr<UserInfo>>&list); // from TcpManager::on_get_apply_list
    void do_add_friend(const UserInfo&info); // from TcpManager::on_add_friend();
    void do_auth_friend(std::shared_ptr<UserInfo>info); // from TcpManager::on_auth_friend
    void do_notifications_to_list(const std::vector<std::shared_ptr<UserInfo>>&list);
    void do_notify_friend(std::shared_ptr<UserInfo>info,bool accept);   // from TcpManager::on_notify_friend
    void do_notify_friend2(std::shared_ptr<UserInfo>info,bool accept);   // from TcpManager::on_notify_friend2
private:
    QListWidget *friendsNews;
    QListWidget *systemNews;
    QPropertyAnimation *slideAnimation;
    QPropertyAnimation *fadeAnimation;
    QGraphicsOpacityEffect *opacityEffect;
    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);

    // QWidget interface
protected:
    void focusOutEvent(QFocusEvent *event);
};

#endif // NOTIFICATIONPANEL_H
