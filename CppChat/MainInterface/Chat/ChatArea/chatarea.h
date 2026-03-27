#ifndef CHATAREA_H
#define CHATAREA_H

#include <QTextEdit>
#include <QToolButton>
#include <QWidget>

class QPushButton;
class InputArea;
class MessageArea;
class ChatArea : public QWidget
{
    Q_OBJECT
public:
    explicit ChatArea(QWidget *parent = nullptr);
    void setupUI();
    void setupConnections();
signals:

private:
    InputArea *inputArea;
    MessageArea *messageArea;
};



#endif // INPUTAREA_H
