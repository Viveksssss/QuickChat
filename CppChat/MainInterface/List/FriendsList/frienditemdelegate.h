#ifndef FRIENDITEMDELEGATE_H
#define FRIENDITEMDELEGATE_H


#include <QObject>
#include <QStyledItemDelegate>

class QPushButton;
class QListView;
class FriendsListPart;
class FriendItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    FriendItemDelegate(QWidget*parent = nullptr,FriendsListPart*list=nullptr);

    // QAbstractItemDelegate interface
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
public:
    QListView*getList();
private:
    void showContextMenu(const QPoint &globalPos, const QModelIndex &index);
    void setupConnections();
    QPushButton* createIconButton(const QString& iconPath, const QString& text, int iconSize);
private:
    FriendsListPart*list;

    QMenu*menu;
    QAction *toTopAction;
    QAction *selectAction;
    QAction *deleteAction;
    // QAbstractItemDelegate interface
signals:
    void on_open_friend_info(int,const QString&,const QString&,int,const QString&);
public slots:
    void do_open_friend_info(int,const QString&,const QString&,int,const QString&);
};

#endif // FRIENDITEMDELEGATE_H
