#include "systemnewsitem.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "../../../../Properties/sourcemanager.h"


SystemNewsItem::SystemNewsItem(bool isReply,int uid,const QString &iconPath, const QString &name, const QString &content, QWidget *parent)
    : QWidget(parent)
    , _uid(uid)
    , _isRely(isReply)

{
    setupUI();
    setConnections();

    nameLabel->setText(name);
    contentLabel->setText(content);
    iconLabel->setPixmap(SourceManager::GetInstance()->getPixmap(iconPath).scaled(40,40));
}

void SystemNewsItem::setupUI()
{
    setFixedSize({200,100});

    QVBoxLayout*main_vlay = new QVBoxLayout(this);
    main_vlay->setContentsMargins(0,10,0,10);
    main_vlay->setSpacing(0);

    iconLabel = new QLabel;
    iconLabel->setFixedSize(40,40);
    iconLabel->setObjectName("FriendsNewItem_IconLabel");

    iconLabel->setAlignment(Qt::AlignCenter);

    // 名称+内容
    QVBoxLayout*text_vlay = new QVBoxLayout;
    text_vlay->setContentsMargins(0,0,0,0);

    nameLabel = new QLabel;
    nameLabel->setObjectName("FriendsNewItem_NameLabel");

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
        button_hlay->addWidget(acceptButton,Qt::AlignCenter);
        button_hlay->addStretch();
    }else{
        acceptButton = new QPushButton;
        acceptButton->setObjectName("FriendsNewsItem_AcceptButton");
        acceptButton->setFixedSize(90,30);

        rejectButton = new QPushButton;
        rejectButton->setObjectName("FriendsNewsItem_RejectButton");
        rejectButton->setFixedSize(90,30);

        button_hlay->addWidget(acceptButton);
        button_hlay->addWidget(rejectButton);
    }


    main_vlay->addLayout(top_hlay);
    main_vlay->addLayout(button_hlay);
}

void SystemNewsItem::setConnections()
{
    if (_isRely){
        connect(acceptButton,&QPushButton::clicked,this,&SystemNewsItem::do_accept_clicked);
    }else{
        connect(acceptButton,&QPushButton::clicked,this,&SystemNewsItem::do_accept_clicked);
        connect(rejectButton,&QPushButton::clicked,this,&SystemNewsItem::do_reject_clcked);
    }
}

void SystemNewsItem::do_accept_clicked()
{
    if (_isRely){
        emit on_confirm_clicked();
    }else{
        emit on_accepted_clicked();
        // tcp发送消息TODO:
    }
}

void SystemNewsItem::do_reject_clcked()
{
    emit on_rejected_clicked();
    // tcp发送消息TODO:
}
