#ifndef SIGNALROUTER_H
#define SIGNALROUTER_H

#include <QObject>
#include "../MainInterface/List/FriendsList/frienditem.h"
#include "../MainInterface/Chat/ChatArea/MessageArea/messagetypes.h"

class SignalRouter : public QObject
{
    Q_OBJECT
public:
    explicit SignalRouter(QObject *parent = nullptr);
    static SignalRouter& GetInstance();
signals:
    void on_init_avatar();  // from MainScreen::init(); // to ChatTopArea::ClearAvatar
    void on_init_profile(); // from MainScreen::init(); // to ChatTopArea::ProfilePopup
    void on_change_profile_status(int status);   // from eventwhere we change profile.      to ChatTopArea::ProfilePopup
    void on_change_friend_status(int,int);          // from FriendsNewsItem::do_accept_clicked
    void on_change_friend_selection(FriendItem);    // from FriendsListPart::QListView::clicked
    void on_change_message_selection(ConversationItem);   // from MessagesListPart::QListView::clicked
    void on_add_message_to_list(ConversationItem);        // from FriendsItem::clicked
    void on_change_list(int);                       // from SideBar::ButtonGroup::clicked
    void on_change_peer(int);                       // from FriendsDelegate::dialog::clicked/MessageItem::clicked
    void on_change_last_time(int,QDateTime);                     // from DataBase::getMessages
    void on_to_list(int);                      // from MessageDelegate::do_change_peer
    void on_message_item(int);                 // from MessageArea::do_change_peer
    void on_add_new_message(const MessageItem&item);    // from MessageListPart::on_get_message
    void on_change_message_status(const MessageItem&item);  // from MessageListPart::on_get_message
    void on_eliminate_status();                 // from ListPart::do_change_list -> to SideBarPart::do_eliminate_status();
    void on_update_avatar(const QString&avatar);    // from ChatTopArea::clicked    -> to ClearAvatarLabel::do_update_avatar();
public slots:

};

#endif // SIGNALROUTER_H
