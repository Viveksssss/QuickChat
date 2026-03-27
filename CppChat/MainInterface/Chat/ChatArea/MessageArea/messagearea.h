#ifndef MESSAGEAREA_H
#define MESSAGEAREA_H

#include <QLabel>
#include <QWidget>

class MessageDelegate;
class MessageModel;
class QListView;
struct MessageItem;
class InputArea;
class MessageArea : public QWidget
{
    Q_OBJECT
public:
    explicit MessageArea(QWidget *parent = nullptr);
    void setupUI();
    void setupConnections();
    MessageModel*getModel();
    void showToast(const QString& message, int duration = 100);
    void setInputArea(InputArea*area);
signals:
    void on_load_more_message();
public slots:
    void do_area_to_bottom(); // from InputArea::on_message_sent
    void do_change_peer(int);    // from SignalRoute::on_change_peer;
    void do_change_chat_history(std::vector<std::shared_ptr<MessageItem>>,bool _delete = false); // from TcpManager::on_change_chat_history;
    void do_change_chat_history(std::vector<MessageItem>,bool _delete = false); // from TcpManager::on_change_chat_history;
    void do_load_more_message();            // fron this::on_load_more_message
    void do_add_new_message(const MessageItem&item);    // from MessageListPart::do_get_message->SignalRouter::on_add_new_message
public:
    QListView *list;
    MessageDelegate*delegate;
    MessageModel*model;
    InputArea *inputArea;

    QLabel *toastLabel = nullptr;

    bool isLoading = false;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // MESSAGEAREA_H
