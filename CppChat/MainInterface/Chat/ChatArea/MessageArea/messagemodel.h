#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include "messagetypes.h"

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MessageRole{
        TypeRole = Qt::UserRole + 1,
        IdRole,
        RecvIdRole,
        SenderRole,
        TimestampRole,
        ContentsRole,
        SelectedRole,
        DisplayTimeRole,
        BubbleColorRole,
        AlignmentRole,
        MessageEnvRole,
    };

    explicit MessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role)override;
    bool removeRows(int row, int count, const QModelIndex &parent)override;

    QHash<int,QByteArray>roleNames()const override;
    void addMessage(const MessageItem&message);
    void addMessage(std::shared_ptr<MessageItem>message);
    void addPreMessage(std::shared_ptr<MessageItem>message);
    void addPreMessage(MessageItem message);
    void addMessage(const QList<MessageItem>&messages);
    void removeMessage(const QString&id);
    void clearMessage();
    void setMessageSelected(const QString &id,bool selected);
    QList<MessageItem>getSelectedMessages()const;
private:
    QList<MessageItem>messages;

};

#endif // MESSAGEMODEL_H
