#ifndef SYSTEMNEWSITEM_H
#define SYSTEMNEWSITEM_H

#include <QObject>
#include <QWidget>

class QLabel;
class QPushButton;

class SystemNewsItem : public QWidget
{
    Q_OBJECT
public:
    explicit SystemNewsItem(bool isReply,int uid,const QString&iconPath,const QString&name,const QString&content,QWidget *parent = nullptr);
private:
    void setupUI();
    void setConnections();
private:
    QLabel *iconLabel;
    QLabel *nameLabel;
    QLabel *contentLabel;
    QPushButton *acceptButton;
    QPushButton *rejectButton;

    int _uid;
    bool _isRely;
private slots:
    void do_accept_clicked();
    void do_reject_clcked();

signals:
    void on_accepted_clicked();
    void on_rejected_clicked();
    void on_confirm_clicked();
};

#endif // SYSTEMNEWSITEM_H
