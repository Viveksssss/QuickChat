#include "friendsnewsitem.h"
#include <QJsonObject>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>
#include "../../../../usermanager.h"
#include "../../../../Properties/signalrouter.h"
#include "../../../../tcpmanager.h"


FriendsNewsItem::FriendsNewsItem(bool isReply,int code,int uid,int sex,const QString &iconPath, const QString &name, const QString &content, QWidget *parent)
    : QWidget(parent)
    , _uid(uid)
    , _isRely(isReply)
    , _sex(sex)
    , _icon(iconPath)
    , _code(code)

{
    setupUI();
    setConnections();

    nameLabel->setText(name);
    contentLabel->setText(content);


    QPixmap originalPixmap;
    // 创建带边框的圆形图片
    if (_icon.startsWith(":/")){
        originalPixmap = QPixmap(_icon);
    }else{
        QByteArray imageData = QByteArray::fromBase64(_icon.toUtf8());
        originalPixmap.loadFromData(imageData);
    }

    QPixmap finalPixmap(44, 44);
    finalPixmap.fill(Qt::transparent);

    QPainter painter(&finalPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 1. 先绘制边框
    QColor borderColor = (_sex == 1) ? QColor("#00F5FF") : QColor("#FF69B4");
    painter.setBrush(borderColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 44, 44);

    // 2. 绘制背景
    painter.setBrush(QColor("#E3F2FD"));
    painter.drawEllipse(2, 2, 40, 40);  // 边框内部

    // 3. 裁剪并绘制头像
    QPainterPath clipPath;
    clipPath.addEllipse(2, 2, 40, 40);  // 头像区域
    painter.setClipPath(clipPath);
    painter.drawPixmap(2, 2, 40, 40, originalPixmap.scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    iconLabel->setPixmap(finalPixmap);
}

void FriendsNewsItem::setupUI()
{
    setFixedSize({250,100});

    QVBoxLayout*main_vlay = new QVBoxLayout(this);
    main_vlay->setContentsMargins(0,10,0,10);
    main_vlay->setSpacing(0);

    iconLabel = new QLabel;
    iconLabel->setFixedSize(44,44);
    iconLabel->setAlignment(Qt::AlignCenter);

    // 名称+内容
    QVBoxLayout*text_vlay = new QVBoxLayout;
    text_vlay->setContentsMargins(0,0,0,0);

    nameLabel = new QLabel;
    nameLabel->setObjectName("FriendsNewItem_NameLabel");
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    contentLabel = new QLabel;
    contentLabel->setObjectName("FriendsNewsItem_ContentLabel");
    text_vlay->addWidget(nameLabel);
    text_vlay->addWidget(contentLabel);

    QHBoxLayout*top_hlay = new QHBoxLayout;
    top_hlay->setContentsMargins(0,0,0,0);
    top_hlay->addWidget(iconLabel);
    top_hlay->addLayout(text_vlay);


    QHBoxLayout*button_hlay = new QHBoxLayout;
    button_hlay->setContentsMargins(0,0,0,0);
    button_hlay->setSpacing(10);
    if (_isRely){
        acceptButton = new QPushButton;
        acceptButton->setObjectName("FriendsNewsItem_AcceptButton");
        acceptButton->setFixedSize(90,30);
        acceptButton->setText("确认");
        button_hlay->addWidget(acceptButton,Qt::AlignCenter);
        button_hlay->addStretch();
    }else{
        acceptButton = new QPushButton;
        acceptButton->setObjectName("FriendsNewsItem_AcceptButton");
        acceptButton->setFixedSize(90,30);
        acceptButton->setText("接受");

        rejectButton = new QPushButton;
        rejectButton->setObjectName("FriendsNewsItem_RejectButton");
        rejectButton->setFixedSize(90,30);
        rejectButton->setText("拒绝");

        button_hlay->addWidget(acceptButton);
        button_hlay->addWidget(rejectButton);
    }


    main_vlay->addLayout(top_hlay);
    main_vlay->addLayout(button_hlay);
}

void FriendsNewsItem::setConnections()
{
    connect(acceptButton,&QPushButton::clicked,this,&FriendsNewsItem::do_accept_clicked);
    if (!_isRely){
        connect(rejectButton,&QPushButton::clicked,this,&FriendsNewsItem::do_reject_clcked);
    }
}

void FriendsNewsItem::do_accept_clicked()
{
    if (_isRely){
        emit on_confirm_clicked();
        if (_code == static_cast<int>(NotificationCodes::ID_NOTIFY_MAKE_FRIENDS)){
            QJsonObject jsonObj;
            jsonObj["from_uid"] = UserManager::GetInstance()->GetUid();
            jsonObj["reply"] = true;
            QJsonDocument doc(jsonObj);
            TcpManager::GetInstance()->do_send_data(RequestType::ID_AUTH_FRIEND_REQ,doc.toJson(QJsonDocument::Compact));
        }else if (_code == static_cast<int>(NotificationCodes::ID_NOTIFY_FRIEND_ONLINE)){
            emit SignalRouter::GetInstance().on_change_friend_status(_uid,1);
        }else if(_code == static_cast<int>(NotificationCodes::ID_NOTIFY_NOT_FRIENDs)){
            QJsonObject jsonObj;
            jsonObj["from_uid"] = UserManager::GetInstance()->GetUid();
            jsonObj["reply"] = true;
            QJsonDocument doc(jsonObj);
            TcpManager::GetInstance()->do_send_data(RequestType::ID_AUTH_FRIEND_REQ,doc.toJson(QJsonDocument::Compact));
        }
    }else{
        emit on_accepted_clicked(); // 提示消除item
        // 下面回复请求为接受
        QJsonObject jsonObj;
        jsonObj["from_uid"] = UserManager::GetInstance()->GetUid();
        jsonObj["to_uid"] = _uid;
        jsonObj["from_name"] = UserManager::GetInstance()->GetName();
        jsonObj["from_sex"] = UserManager::GetInstance()->GetSex();
        jsonObj["from_icon"] =UserManager::GetInstance()->GetIcon();
        jsonObj["from_desc"] =UserManager::GetInstance()->GetDesc();
        jsonObj["accept"] = true;
        QJsonDocument doc(jsonObj);
        TcpManager::GetInstance()->do_send_data(RequestType::ID_AUTH_FRIEND_REQ,doc.toJson(QJsonDocument::Compact));
    }
}

void FriendsNewsItem::do_reject_clcked()
{
    emit on_rejected_clicked();
    // 下面回复请求为拒绝
    QJsonObject jsonObj;
    jsonObj["from_uid"] = UserManager::GetInstance()->GetUid();
    jsonObj["to_uid"] = _uid;
    jsonObj["from_name"] = UserManager::GetInstance()->GetName();
    jsonObj["from_sex"] = UserManager::GetInstance()->GetSex();
    jsonObj["from_icon"] =UserManager::GetInstance()->GetIcon();
    jsonObj["from_desc"] =UserManager::GetInstance()->GetDesc();
    jsonObj["accept"] = false;
    QJsonDocument doc(jsonObj);
    TcpManager::GetInstance()->do_send_data(RequestType::ID_AUTH_FRIEND_REQ,doc.toJson(QJsonDocument::Compact));
}
