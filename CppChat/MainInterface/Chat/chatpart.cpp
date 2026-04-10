#include "chatpart.h"
#include "ChatTopArea/chattoparea.h"
#include "ChatArea/chatarea.h"
#include <QVBoxLayout>
#include <QLabel>

ChatPart::ChatPart(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
    setupConnections();
}

void ChatPart::setupUI()
{
    setContentsMargins(0,0,0,0);
    setMinimumWidth(300);
    setMinimumHeight(200);
    setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout *main_vlay = new QVBoxLayout(this);
    main_vlay->setContentsMargins(10,10,10,10);

    // chatTopArea
    chatTopArea = new ChatTopArea(this);
    // chatArea
    chatArea = new ChatArea;

    main_vlay->addWidget(chatTopArea);
    main_vlay->addWidget(chatArea);

}

void ChatPart::setupConnections()
{

}
