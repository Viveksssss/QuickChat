#include "messageitemdelegate.h"
#include "../FriendsList/frienditem.h"
<<<<<<< HEAD
#include "../../../Properties/signalrouter.h"
=======
#include "../../../../Properties/signalrouter.h"
>>>>>>> origin/main
#include "messageslistpart.h"
#include "messagesmodel.h"
#include <QFile>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>


MessageItemDelegate::MessageItemDelegate(QWidget* parent,MessagesListPart*list)
    : QStyledItemDelegate(parent)
    , list(list)
{
    menu = new QMenu();
    toTopAction = new QAction("置顶",menu);
    selectAction = new QAction("选择",menu);
    deleteAction = new QAction("删除",menu);

    menu->addAction(toTopAction);
    menu->addAction(selectAction);
    menu->addAction(deleteAction);
}

void MessageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)const
{
    painter->save();
    // 1. 绘制背景
    QRect rect = option.rect;
    int radius = 12;
    QColor bg;
    QPen pen = Qt::NoPen;
    if(option.state & QStyle::State_Selected){
        bg = QColor(225,240,255);
        pen = QPen(QColor(0xc1e6d9),2);
    }else if(option.state & QStyle::State_MouseOver){
        bg = QColor(245,245,245);
    }else{
        bg = QColor(0xf9fafb);
    }


    painter->setPen(pen);
    painter->setBrush(bg);
    painter->drawRoundedRect(rect, radius, radius);

    // 2. 获取数据
    QString name = index.data(MessagesModel::NameRole).toString();
    QString avatarStr = index.data(MessagesModel::AvatarRole).toString();
    int status = index.data(MessagesModel::StatusRole).toInt();
    QString message = index.data(MessagesModel::MessageRole).toString();
    bool processed = index.data(MessagesModel::RedDotRole).toBool();

    // 3. 绘制头像
    QRect avatarRect(rect.left() + 10, rect.top() + 10, 40, 40);
    QPixmap avatar;
    if(!avatarStr.isEmpty() ){
        if (avatarStr.startsWith(":/")){
            avatar = QPixmap(avatarStr).scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        }else{
            // base64
            QByteArray imageData = QByteArray::fromBase64(avatarStr.toUtf8());
            avatar.loadFromData(imageData);
        }
    }else{
        avatar = QPixmap(":/Resources/main/header-default.png").scaled(40,40);
        // avatar.fill(QColor(200,200,200));
    }

    // 绘制红点
    if (!processed){
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 红点位置：头像右上角，稍微向内偏移
        int redDotSize = 10; // 红点大小
        int offset = 2; // 向内偏移量，避免紧贴边缘

        QPoint redDotCenter(
            avatarRect.right() - redDotSize/2 + offset,
            avatarRect.top() + redDotSize/2 - offset
            );

        QRect redDotRect(
            redDotCenter.x() - redDotSize/2,
            redDotCenter.y() - redDotSize/2,
            redDotSize,
            redDotSize
            );

        // 绘制红点
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 0, 0)); // 红色
        painter->drawEllipse(redDotRect);

        painter->restore();
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath mask;
    mask.addEllipse(avatarRect);
    painter->setClipPath(mask);
    painter->drawPixmap(avatarRect, avatar);
    painter->restore();

    // 4. 状态标记 - 修正后的布局计算
    QColor statusColor;
    QString statusStr;
    if(status == 1) {
        statusColor = QColor(0x58f376);
        statusStr = "在线";
    }

    else if(status == 2){
        statusColor = QColor(0xe90739);
        statusStr = "忙碌";
    }
    else if(status == 0){
        statusColor = QColor(0x51615f);
        statusStr = "离线";
    }
    else{
        statusColor = QColor(0x51615f); // 默认颜色
        statusStr = "离线";
    }

    QFontMetrics fm(painter->font());
    int textWidth = fm.horizontalAdvance(statusStr);
    int dotSize = 8;  // 圆点大小
    int spacing = 5;  // 圆点和文字间距

    // 正确计算状态区域的位置
    int statusTotalWidth = dotSize + spacing + textWidth;
    int statusX = rect.right() - statusTotalWidth - 15;  // 右边距15
    int statusY = rect.top() + 27;  // 从顶部开始

    // 绘制状态圆点
    // QRect dotRect(statusX, statusY, dotSize, dotSize);
    // painter->save();
    // painter->setRenderHint(QPainter::Antialiasing);
    // painter->setPen(Qt::NoPen);
    // painter->setBrush(statusColor);
    // painter->drawEllipse(dotRect);
    // painter->restore();

    // 绘制状态文字
    // painter->save();
    // painter->setPen(statusColor);
    // QFont statusFont = painter->font();
    // statusFont.setPointSize(8);
    // painter->setFont(statusFont);
    // QRect statusTextRect(dotRect.right() + spacing, statusY - dotSize/2, textWidth, dotSize*2);
    // painter->drawText(statusTextRect, Qt::AlignLeft | Qt::AlignVCenter, statusStr);
    // painter->restore();

    // 5. 绘制昵称 - 修正宽度计算
    painter->save();
    painter->setPen(Qt::black);
    QFont nameFont = painter->font();
    nameFont.setPointSize(11);
    nameFont.setBold(true);
    painter->setFont(nameFont);

    // 昵称区域的宽度要避开状态区域
    int nameMaxWidth = statusX - avatarRect.right() - 10; // 减去间距
    QRect nameRect(avatarRect.right() + 15, rect.top() + 12, nameMaxWidth, 20);
    QString elidedName = painter->fontMetrics().elidedText(name, Qt::ElideRight, nameRect.width());
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);
    painter->restore();

    // 6. 绘制消息
    painter->save();
    painter->setPen(QColor(128,128,128));
    QFont msgFont = painter->font();
    msgFont.setPointSize(9);
    painter->setFont(msgFont);

    QRect msgRect(avatarRect.right() + 15, nameRect.bottom() + 2, nameMaxWidth, 16);
    QString elidedMessage = painter->fontMetrics().elidedText(message, Qt::ElideRight, msgRect.width());
    painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignVCenter, elidedMessage);
    painter->restore();

    painter->restore();
}

bool MessageItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton){
            showContextMenu(mouseEvent->globalPosition().toPoint(), index);
            return true;
        }else if(mouseEvent->button() == Qt::LeftButton){
            change_info(index);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize MessageItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(option.rect.width(),60);
}

QListView *MessageItemDelegate::getList()
{
    return list->getList();
}

void MessageItemDelegate::change_info(const QModelIndex &index)
{
    UserManager::GetInstance()->setHistoryTimestamp(UserManager::GetInstance()->GetPeerUid(),QDateTime::currentDateTime());

    // int env = index.data(MessagesModel::MessageEnvRole).toInt();
    int peerUid =index.data(MessagesModel::ToUidRole).toInt();
    // UserManager::GetInstance()->SetPeerUid(peerUid);
    list->do_change_message_status(peerUid,true);

    int peerUU = UserManager::GetInstance()->GetPeerUid();
    if (peerUid == UserManager::GetInstance()->GetPeerUid()){
        return;
    }
    UserManager::GetInstance()->ChangeUserInfo(peerUid);

    emit SignalRouter::GetInstance().on_change_peer(peerUid);
}

QPixmap MessageItemDelegate::getStatusPximap(const QString &status) const
{
}

void MessageItemDelegate::showContextMenu(const QPoint &globalPos, const QModelIndex &index)
{
    QAction *selectedAction = menu->exec(globalPos);
    if (!selectedAction || !index.isValid())
        return;
    QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
    if (!model){
        return;
    }

    if (selectedAction == toTopAction){
        int pos = index.row();
        if (pos == 0){
            return;
        }else{
            bool ok = model->moveRow(QModelIndex(), pos,QModelIndex(), 0);
        }
    }else if(selectedAction == selectAction){
        auto *p= list->getList();
        if(p){
            p->setCurrentIndex(index);
        }
    }else if(selectedAction == deleteAction){
        model->removeRow(index.row(),QModelIndex());
    }
}

