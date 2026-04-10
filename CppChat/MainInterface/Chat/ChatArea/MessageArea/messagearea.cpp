#include "messagearea.h"
#include <QListView>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QJsonObject>
#include <QEvent>
#include <QWheelEvent>
#include <QTimer>
#include "./messagemodel.h"
#include "./messagedelegate.h"
#include "../../../../Properties/signalrouter.h"
#include "../../../../tcpmanager.h"
#include "../../../../database.h"

MessageArea::MessageArea(QWidget *parent)
    : QWidget{parent}
    , list(new QListView(this))
    , delegate(new MessageDelegate(this))
    , model(new MessageModel(this))
{
    delegate->setMessageArea(this);

    setupUI();
    setupConnections();

    do_load_more_message();
}

void MessageArea::setupUI()
{
    QVBoxLayout *main_vLay = new QVBoxLayout(this);
    main_vLay->setContentsMargins(0,0,0,0);
    main_vLay->setSpacing(0);
    main_vLay->addWidget(list);

    list->setObjectName("messageList");
    list->setModel(model);
    list->setItemDelegate(delegate);
    list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    list->setAlternatingRowColors(false);
    list->verticalScrollBar()->setSingleStep(20);
    list->setUniformItemSizes(false);   // 必须 false，才允许每行高不同
    list->setWordWrap(true);            // 内部也会触发 QTextLayout 折行
    list->setSelectionMode(QAbstractItemView::NoSelection);
    list->setFocusPolicy(Qt::ClickFocus);

    list->viewport()->installEventFilter(this);

    // 在 setupUI() 中添加：
    list->setFrameStyle(QFrame::NoFrame); // 移除边框
}

void MessageArea::setupConnections()
{
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_change_peer,this,&MessageArea::do_change_peer);
    connect(this,&MessageArea::on_load_more_message,this,&MessageArea::do_load_more_message);
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_add_new_message,this,&MessageArea::do_add_new_message);
}

MessageModel *MessageArea::getModel()
{
    return this->model;
}

void MessageArea::do_area_to_bottom()
{
    list->scrollToBottom();
}

void MessageArea::do_change_peer(int uid)
{
    model->clearMessage();

    const auto&historys = DataBase::GetInstance().getMessages(uid,QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    do_change_chat_history(historys,true);

    // emit SignalRouter::GetInstance().on_message_item(uid);
    do_area_to_bottom();
    list->update();
    list->repaint();  // 添加这个
    list->viewport()->update();  // 还有这个

    // QJsonObject j ;
    // j["from_uid"] = UserManager::GetInstance()->GetUid();
    // j["to_uid"] = uid;
    // // j["timestamp"]
    // QJsonDocument doc(j);
    // TcpManager::GetInstance()->on_send_data(RequestType::ID_GET_MESSAGES_OF_FRIEND_REQ,doc.toJson());
}

void MessageArea::do_change_chat_history(std::vector<std::shared_ptr<MessageItem>>messages,bool _delete)
{
    if (_delete){
        model->clearMessage();
    }
    for (const auto&message:messages){
        model->addPreMessage(message);
    }
}

void MessageArea::do_change_chat_history(std::vector<MessageItem>messages,bool _delete)
{
    if (_delete){
        model->clearMessage();
    }
    for (const auto&message:messages){
        model->addPreMessage(message);
    }
}

void MessageArea::do_load_more_message()
{
    if(isLoading){
        return;
    }
    isLoading = true;

    QString timestamp =UserManager::GetInstance()->GetHistoryTimestamp(UserManager::GetInstance()->GetPeerUid()).toString("yyyy-MM-dd HH:mm:ss");
    const auto&historys = DataBase::GetInstance().getMessages(UserManager::GetInstance()->GetPeerUid(),timestamp);
    do_change_chat_history(historys,false);
    if (historys.size()>0){
        showToast(QString("loading..."));
    }


    QTimer::singleShot(1000,this,[this](){
        this->isLoading = false;
    });
}

void MessageArea::showToast(const QString& message, int duration)
{
    if (!toastLabel) {
        toastLabel = new QLabel(this);
        toastLabel->setAlignment(Qt::AlignCenter);
        toastLabel->setStyleSheet(
            "background-color: rgba(0, 0, 0, 180);"
            "color: white;"
            "padding: 8px 16px;"
            "border-radius: 4px;"
            "font-size: 14px;"
            );
        toastLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        toastLabel->hide();

        // 关键：创建一个覆盖层布局
        QVBoxLayout* overlayLayout = new QVBoxLayout(this);
        overlayLayout->setContentsMargins(0, 0, 0, 0);
        overlayLayout->addWidget(toastLabel, 0, Qt::AlignCenter);

        // 确保主布局存在
        if (!layout()) {
            setLayout(overlayLayout);
        } else {
            // 如果已有布局，添加一个覆盖层
            QWidget* overlay = new QWidget(this);
            overlay->setLayout(overlayLayout);
            overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
            overlay->setStyleSheet("background: transparent;");
            overlay->show();
        }
    }

    toastLabel->setText(message);
    toastLabel->adjustSize();

    // 强制更新几何
    updateGeometry();

    // 使用 QMetaObject 延迟调用，确保几何已更新
    QMetaObject::invokeMethod(this, [this]() {
        if (toastLabel && toastLabel->isVisible()) {
            toastLabel->raise();  // 确保在最上层
        }
    }, Qt::QueuedConnection);

    toastLabel->show();

    QTimer::singleShot(duration, toastLabel, &QLabel::hide);
}

void MessageArea::setInputArea(InputArea *area)
{
    this->inputArea = area;
}

void MessageArea::do_add_new_message(const MessageItem &item)
{
    model->addMessage(item);
    do_area_to_bottom();
}

void MessageArea::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

bool MessageArea::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == list->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        QScrollBar *vScrollBar = list->verticalScrollBar();
        if (vScrollBar) {
            // 自定义滚动步长
            int delta = wheelEvent->angleDelta().y();
            int step = delta > 0 ? -30 : 30;  // 反向，因为滚动条值增加是向下
            vScrollBar->setValue(vScrollBar->value() + step);

            int minimum = vScrollBar->minimum();
            int currentValue = vScrollBar->value();
            if (currentValue <= minimum+10 && !UserManager::GetInstance()->IsLoadMessagesFinished(UserManager::GetInstance()->GetPeerUid())){
                emit on_load_more_message();
            }

            return true; // 事件已处理
        }
    }
    return QWidget::eventFilter(obj, event);
}
