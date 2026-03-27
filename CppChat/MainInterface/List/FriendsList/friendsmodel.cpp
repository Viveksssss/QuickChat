#include "friendsmodel.h"

#include <QLabel>

FriendsModel::FriendsModel(QObject *parent)
    : QAbstractListModel{parent}
{}

int FriendsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : _friends.size();
}

QVariant FriendsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= _friends.size()){
        return QVariant();
    }

    const FriendItem&friendItem = _friends.at(index.row());

    switch(role){
    case Qt::DisplayRole:
        return QString("%1(%2)").arg(friendItem.name).arg(friendItem.status);
    case NameRole:
        return friendItem.name;
    case IdRole:
        return friendItem.id;
    case AvatarRole:
        return friendItem.avatar;
    case StatusRole:
        return friendItem.status;
    case DescRole:
        return friendItem.desc;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FriendsModel::roleNames() const
{
    QHash<int ,QByteArray>roles;
    roles[IdRole] = "friendId";
    roles[NameRole] = "friendName";
    roles[AvatarRole] = "avatar";
    roles[StatusRole] = "status";
    roles[DescRole] = "desc";
    return roles;
}

void FriendsModel::addFriend(const FriendItem &friendItem)
{
    beginInsertRows(QModelIndex(),_friends.size(),_friends.size());
    _friends.append(friendItem);
    endInsertRows();
}

void FriendsModel::addPreFriend(const FriendItem &friendItem)
{
    beginInsertRows(QModelIndex(),0,0);
    _friends.append(friendItem);
    endInsertRows();
}

FriendItem FriendsModel::getFriend(int index)
{
    if (index >= 0 && index < _friends.size()){
        return _friends.at(index);
    }
    return FriendItem();
}

QModelIndex FriendsModel::indexFromUid(int uid) const
{
    // 遍历所有行，检查uid角色
    for (int row = 0; row < rowCount(); ++row) {
        QModelIndex index = createIndex(row, 0);
        if (data(index, IdRole).toInt() == uid) {
            return index;
        }
    }
    return QModelIndex();
}

QVector<FriendItem> &FriendsModel::getList()
{
    return this->_friends;
}

bool FriendsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row > _friends.size()){
        return false;
    }
    beginRemoveRows(parent,row,row+count-1);
    _friends.remove(row,count);
    endRemoveRows();
    return true;
}

bool FriendsModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent.isValid() || destinationParent.isValid() || count != 1){
        return false;
    }
    if (sourceRow == destinationChild){
        return false;
    }
    if (!(sourceRow >= 0 &&sourceRow <= _friends.size() &&destinationChild>=0 && destinationChild<=_friends.size())){
        return false;
    }

    beginMoveRows(sourceParent,sourceRow,sourceRow+count-1,destinationParent,destinationChild);
    if (sourceRow < destinationChild){
        _friends.insert(destinationChild,_friends.at(sourceRow));
    }else{
        _friends.insert(destinationChild, _friends.at(sourceRow));
        _friends.remove(sourceRow + 1);   // 因为刚插完index+1
    }
    endMoveRows();
    return true;
}

bool FriendsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= _friends.size()){
        return false;
    }

    FriendItem& friendItem = _friends[index.row()];

    switch(role){
    case Qt::DisplayRole:
        break;
    case NameRole:
        friendItem.name = value.toString();
        break;
    case IdRole:
        friendItem.id = value.toInt();
        break;
    case AvatarRole:
        friendItem.avatar = value.toString();
        break;
    case StatusRole:
        friendItem.status = value.toInt();
        break;
    default:
        return false;
    }
    emit dataChanged(index, index, {role});
    return true;
}

