#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include <QObject>
#include <QWidget>

class ChatPart;
class ListPart;
class SideBarPart;
class TopTitlePart;
class MainScreen : public QWidget
{
    Q_OBJECT
public:
    explicit MainScreen(QWidget *parent = nullptr);
    void init();
private:
    // 设置ui
    void setupUI();
    // 设置信号与槽
    void setupConnections();

signals:


private:
    // 聊天区域
    ChatPart *chatPart;
    // 好友列表
    ListPart *listPart;
    // 左侧边栏
    SideBarPart *sideBarPart;
    // 顶部边栏
    TopTitlePart *topTitlePart;
};

#endif // MAINSCREEN_H
