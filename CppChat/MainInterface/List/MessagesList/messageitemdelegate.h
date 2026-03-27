#ifndef MESSAGEITEMDELEGATE_H
#define MESSAGEITEMDELEGATE_H


#include <QObject>
#include <QStyledItemDelegate>

class QListView;
class MessagesListPart;



class MessageItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MessageItemDelegate(QWidget*parent = nullptr,MessagesListPart*list=nullptr);

    // QAbstractItemDelegate interface
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QListView*getList();

    void change_info(const QModelIndex &index);

private:
    QPixmap getStatusPximap(const QString &status)const;
    void showContextMenu(const QPoint &globalPos, const QModelIndex &index);
private:

    MessagesListPart*list;

    QMenu*menu;
    QAction *toTopAction;
    QAction *selectAction;
    QAction *deleteAction;

};

#endif // MESSAGEITEMDELEGATE_H
