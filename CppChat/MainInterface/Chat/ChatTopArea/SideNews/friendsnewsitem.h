#ifndef FRIENDSNEWSITEM_H
#define FRIENDSNEWSITEM_H

#include <QObject>
#include <QWidget>

class QLabel;
class QPushButton;

class FriendsNewsItem : public QWidget
{
    Q_OBJECT
public:
    explicit FriendsNewsItem(bool isReply,int code,int uid,int sex,const QString&iconPath,const QString&name,const QString&content,QWidget *parent = nullptr);
private:
    void setupUI();
    void setConnections();
private:
    QLabel *iconLabel;
    QLabel *nameLabel;
    QLabel *contentLabel;
    QPushButton *acceptButton;
    QPushButton *rejectButton;

    int _uid;   // uid
    bool _isRely; // 判断是好友申请还是申请回复，申请需要两个按钮，回复只需要一个按钮：使用acceptButton代替确认
    int _sex;   // 性别
    QString _icon;  // 图标
    int _code; // 通知类型
private slots:
    void do_accept_clicked();
    void do_reject_clcked();

signals:
    void on_accepted_clicked();
    void on_rejected_clicked();
    void on_confirm_clicked();

};

#endif // FRIENDSNEWSITEM_H
