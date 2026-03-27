#ifndef FRIENDSMODEL_H
#define FRIENDSMODEL_H

#include "frienditem.h"

#include <QAbstractListModel>
#include <QObject>

class FriendsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum FriendRole{
        IdRole = Qt::UserRole + 1,  // id
        NameRole,                   // 昵称
        AvatarRole,                 // 头像
        StatusRole,                 // 状态
        DescRole,                   // 个性签名
    };

    explicit FriendsModel(QObject *parent = nullptr);
    // QAbstractItemModel interface
    int rowCount(const QModelIndex&parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int,QByteArray>roleNames()const override;

    void addFriend(const FriendItem&friendItem);
    void addPreFriend(const FriendItem&friendItem);
    FriendItem getFriend(int index);

    // 在 FriendsModel 类中添加方法
    QModelIndex indexFromUid(int uid) const;

    QVector<FriendItem>&getList();

private:
    QVector<FriendItem>_friends;

    // QAbstractItemModel interface
public:
    bool removeRows(int row, int count, const QModelIndex &parent)override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)override;
    bool setData(const QModelIndex &index, const QVariant &value, int role)override;
};

#endif // FRIENDSMODEL_H
