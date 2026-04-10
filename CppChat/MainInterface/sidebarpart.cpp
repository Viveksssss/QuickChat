#include "sidebarpart.h"
#include "../stylemanager.h"
#include "../Properties/signalrouter.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QAbstractButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QEvent>
#include <QMouseEvent>

SideBarPart::SideBarPart(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
    setupConnections();
}

void SideBarPart::setupUI()
{
    setMinimumWidth(55);
    setMaximumWidth(55);

    setFocusPolicy(Qt::ClickFocus);   // 允许鼠标点击时自己拿焦点

    layout =  new QVBoxLayout(this);
    layout->setContentsMargins(-10,0,0,0);
    layout ->setAlignment(Qt::AlignTop);
    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true); // 互斥选择

    addItem("message", "消息", ":/Resources/main/message.png");
    addItem("friends", "好友", ":/Resources/main/friends.png");


    this->installEventFilter(this);
}

void SideBarPart::setupConnections()
{
    connect(buttonGroup, &QButtonGroup::idClicked, this, [this](int id) {
        emit SignalRouter::GetInstance().on_change_list(id);
    });
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_change_list,this,[this](){
        buttonGroup->setExclusive(false);  // 先取消互斥
        buttonGroup->setExclusive(true);   // 再恢复互斥，这样会清除选中状态
    });
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_eliminate_status,this,&SideBarPart::do_eliminate_status);
}


void SideBarPart::addItem(const QString &id, const QString &text, const QString &icon)
{
    SideBarItem item{id,text,icon,static_cast<int>(items.size())};
    items.append(item);
    createButton(item);
}

void SideBarPart::removeItem(const QString &id)
{
    items.erase(std::remove_if(items.begin(),items.end(),[&id](SideBarItem item){
        return id == item.id;
    }));
}

void SideBarPart::createButton(const SideBarItem &item,bool showText)
{
    QPushButton *btn = new QPushButton;
    btn->setFixedWidth(45);

    // 设置按钮文本和图标
    if (!item.icon.isEmpty()) {
        btn->setIcon(QIcon(item.icon));
        btn->setIconSize(QSize(20, 20));
    }
    if(showText){
        btn->setText(item.text);
    }
    btn->clearFocus();

    // 设置样式
    btn->setStyleSheet(R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            border: none;
            background-color: transparent;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #c7e4fa;
            border-radius:15px;
        }
        QPushButton:checked {
            background-color: #5dd3fe;
            border-radius:15px;
            color: white;
        }
    )");

    btn->setCheckable(true);
    btn->setProperty("itemId", item.id); // 存储标识

    // 添加到布局和按钮组
    layout->insertWidget(layout->count() , btn); // 插入到弹簧之前
    buttonGroup->addButton(btn,nextId++);

    buttons[item.id] = btn;

}

void SideBarPart::do_eliminate_status()
{
    buttonGroup->setExclusive(false);
    // buttonGroup->checkedButton()->setChecked(false);
    for (auto* button : buttonGroup->buttons()) {
        button->setChecked(false);
    }
    buttonGroup->setExclusive(true);
}

bool SideBarPart::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // 检查点击是否在按钮上
        bool clickOnButton = false;
        if (buttonGroup) {
            for (QAbstractButton *button : buttonGroup->buttons()) {
                QPushButton *pushButton = qobject_cast<QPushButton*>(button);
                if (pushButton && pushButton->rect().contains(pushButton->mapFromGlobal(mouseEvent->globalPosition().toPoint()))) {
                    clickOnButton = true;
                    break;
                }
            }
            // 如果点击不在任何按钮上且有选中按钮，则取消选中
            if (!clickOnButton && buttonGroup->checkedButton()) {
                buttonGroup->setExclusive(false);
                buttonGroup->checkedButton()->setChecked(false);
                buttonGroup->setExclusive(true);
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}
