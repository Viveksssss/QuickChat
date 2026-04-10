#ifndef LISTPART_H
#define LISTPART_H

#include <QWidget>
#include "FriendsList/friendslistpart.h"
#include "MessagesList/messageslistpart.h"
#include <QStackedWidget>

class ListPart : public QWidget
{
    Q_OBJECT
public:
    explicit ListPart(QWidget *parent = nullptr);
    void setupUI();
    void setupConnections();
public slots:
    void do_change_list(int);       // from SinglaRouter::on_change_list
private:
    QStackedWidget   *stack;
    FriendsListPart  *friend_part;
    MessagesListPart *message_part;
signals:
};

#endif // LISTPART_H
