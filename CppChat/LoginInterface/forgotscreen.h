#ifndef FORGOTSCREEN_H
#define FORGOTSCREEN_H

#include <QWidget>
#include "../Properties/global.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QFormLayout;
class ForgotScreen : public QWidget
{
    Q_OBJECT
public:
    explicit ForgotScreen(QWidget *parent = nullptr);
    void setupUI();
    void setupConnections();

private:
    QFormLayout *formLayout;

    QLabel *headerLabel;
    QLabel *labelForHeader;

    QLineEdit *accountEdit;
    QLineEdit *emailEdit;

    QLineEdit *securityCode;
    QPushButton *getSecurityCode;

    QLineEdit *newPasswordEdit;
    QLineEdit *confirmNewPasswordEdit;

    QAction *showNewPwdAction;
    QAction *showConfirmPwdAction;

    QPushButton *acceptBtn;
    QPushButton *cancleBtn;

    QHash<RequestType,std::function<void(const QJsonObject&)>> _handlers;
    int countdown = 10;
    QTimer *timer;
private:
    // 定时改变获取按钮的文字
    void do_change_get_code_btn();

    // 获取验证码
    void do_get_code_clicked();
    void do_get_code_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
    // 注册
    void do_forgot_clicked();
    void do_forgot_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);

public:
    void showTip(int code,const QString &str="忘记密码");
    bool doVerify(bool includingSecurityCode=false);
    void initHttpHandlers();
signals:
    void goLogin();
    void readyGoLogin();
};

#endif // FORGOTSCREEN_H
