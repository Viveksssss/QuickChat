#include "messagemodel.h"

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel{parent}
{}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return messages.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= messages.size()){
        return QVariant();
    }

    const MessageItem&message = messages.at(index.row());
    switch(role){
    case TypeRole:
        return static_cast<int>(message.content.type);
    case IdRole:
        return message.id;
    case RecvIdRole:
        return message.to_id;
    case SenderRole:
        return message.from_id;
    case TimestampRole:
        return message.timestamp;
    case ContentsRole:
        return QVariant::fromValue(message.content);
    case SelectedRole:
        return message.isSelected;
    case DisplayTimeRole:
        return message.timestamp.toString("hh:mm");
    case BubbleColorRole:
        return (message.from_id == UserManager::GetInstance()->GetUid())?"#95EC69":"#57feff";
    case AlignmentRole:
        return message.from_id == UserManager::GetInstance()->GetUid()? Qt::AlignRight : Qt::AlignLeft;
    case MessageEnvRole:
        return static_cast<int>(message.env);
    default:
        return QVariant();
    }
}

bool MessageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= messages.size())
        return false;

    MessageItem &message = messages[index.row()];
    bool changed = false;

    switch (role) {
    case SelectedRole:
        if (message.isSelected != value.toBool()) {
            message.isSelected = value.toBool();
            changed = true;
        }
        break;

    case TypeRole:
        message.content.type = static_cast<MessageType>(value.toInt());
        changed = true;
        break;

    case IdRole:
        message.id = value.toString();
        changed = true;
        break;

    case RecvIdRole:
        message.to_id = value.toInt();
        changed = true;
        break;

    case SenderRole:
        message.from_id = value.toInt();
        changed = true;
        break;

    case TimestampRole:
        message.timestamp = value.toDateTime();
        changed = true;
        break;

    case ContentsRole:
        if (value.canConvert<MessageContent>()) {
            message.content = value.value<MessageContent>();
            changed = true;
        }
        break;

    case MessageEnvRole:
        message.env = static_cast<MessageEnv>(value.toInt());
        changed = true;
        break;

    // DisplayTimeRole, BubbleColorRole, AlignmentRole 是计算属性，不需要设置
    case DisplayTimeRole:
    case BubbleColorRole:
    case AlignmentRole:
        // 这些是只读的计算属性，不进行设置
        return false;

    default:
        return false;
    }

    if (changed) {
        emit dataChanged(index, index, {role});
        return true;
    }

    return false;
}

bool MessageModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > messages.size()) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        messages.removeAt(row);
    }
    endRemoveRows();
    return true;
}


QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {
        {TypeRole,"type"},
        {IdRole,"id"},
        {RecvIdRole,"recvId"},
        {SenderRole,"senderid"},
        {TimestampRole,"timestamp"},
        {ContentsRole,"contents"},
        {SelectedRole,"isSelected"},
        {DisplayTimeRole,"displayTime"},
        {BubbleColorRole,"bubbleColor"},
        {AlignmentRole,"alignment"},
        {MessageEnvRole,"env"},
    };
}

void MessageModel::addMessage(const MessageItem &message)
{
    beginInsertRows(QModelIndex(),messages.size(),messages.size());
    messages.append(message);
    endInsertRows();
}

void MessageModel::addMessage(std::shared_ptr<MessageItem> message)
{
    beginInsertRows(QModelIndex(),messages.size(),messages.size());
    messages.append(*message);
    endInsertRows();
}

void MessageModel::addPreMessage(std::shared_ptr<MessageItem> message)
{
    if (!message) return;
    beginInsertRows(QModelIndex(), 0, 0);
    messages.insert(0, *message);
    endInsertRows();
}

void MessageModel::addPreMessage(MessageItem message)
{
    beginInsertRows(QModelIndex(), 0, 0);
    messages.insert(0, message);
    endInsertRows();
}

void MessageModel::addMessage(const QList<MessageItem> &_messages)
{
    if (messages.isEmpty())return;
    beginInsertRows(QModelIndex(),messages.size(),messages.size());
    messages.append(_messages);
    endInsertRows();
}

void MessageModel::removeMessage(const QString &id)
{
    for (int i = 0; i < messages.size(); ++i) {
        if (messages.at(i).id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            messages.removeAt(i);
            endRemoveRows();
            break;
        }
    }
}

void MessageModel::clearMessage()
{
    beginResetModel();
    messages.clear();
    endResetModel();
}

void MessageModel::setMessageSelected(const QString &id, bool selected)
{
    for (int i = 0; i < messages.size(); ++i) {
        if (messages.at(i).id == id) {
            messages[i].isSelected = selected;
            QModelIndex index = createIndex(i, 0);
            emit dataChanged(index, index, {SelectedRole});
            break;
        }
    }
}

QList<MessageItem> MessageModel::getSelectedMessages() const
{
    QList<MessageItem> selected;
    for (const auto &message : messages) {
        if (message.isSelected) {
            selected.append(message);
        }
    }
    return selected;
}
