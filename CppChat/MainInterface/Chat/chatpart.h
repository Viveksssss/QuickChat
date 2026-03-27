#ifndef CHATPART_H
#define CHATPART_H


/******************************************************************************
 *
 * @file       chatpart.h
 * @brief      聊天区域类
 *
 * @author     Vivek
 * @date       2025/10/30
 * @history
 *****************************************************************************/

#include <QWidget>

class ChatTopArea;
class ChatArea;
class ChatPart : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPart(QWidget *parent = nullptr);
private:
    void setupUI();
    void setupConnections();

private:
    ChatTopArea *chatTopArea;
    ChatArea *chatArea;
};

#endif // CHATPART_H
