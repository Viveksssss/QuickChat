#include "chatarea.h"

#include "InputArea/inputarea.h"
#include "MessageArea/messagearea.h"

#include <QEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
ChatArea::ChatArea(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
    setupConnections();

}

void ChatArea::setupUI()
{
    setAttribute(Qt::WA_WindowPropagation);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 移除边距

    // splitter
    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("splitter");
    splitter->setMouseTracking(true);

    // 消息区
    messageArea =new MessageArea;

    // 发送区域
    inputArea= new InputArea;
    inputArea->setModel(messageArea->getModel());

    splitter->addWidget(messageArea);
    splitter->addWidget(inputArea);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false); // 防止子部件被完全拖拽隐藏[citation:1]
    splitter->setStretchFactor(0,5);
    splitter->setStretchFactor(1,1);

    mainLayout->addWidget(splitter);

    // 注入
    messageArea->setInputArea(inputArea);
}

void ChatArea::setupConnections()
{
    connect(inputArea,&InputArea::on_message_sent,messageArea,&MessageArea::do_area_to_bottom);
}

