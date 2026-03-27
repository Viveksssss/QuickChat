#ifndef REGISTERSCREEN_H
#define REGISTERSCREEN_H

#include <QWidget>
#include <QHash>
#include <functional>
#include "../Properties/global.h"
#include <QObject>


class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QFormLayout;
class RegisterScreen : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterScreen(QWidget *parent = nullptr);

signals:
    void goLogin();
    void readyGoLogin();

private:
    void setupUI();
    void setupConnections();
    void showTip(int code,const QString &str="注册账号");
    bool doVerify(bool includingSecurityCode=false);
    void initHttpHandlers();
private slots:
    // 定时改变获取按钮的文字
    void do_change_get_code_btn();

    // 获取验证码
    void do_get_code_clicked();
    void do_get_code_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
    // 注册
    void do_register_clicked();
    void do_register_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
private:

    QHash<RequestType,std::function<void(const QJsonObject&)>> _handlers;
    QTimer*timer;
    int countdown;

    QFormLayout *formLayout;

    //顶层
    QLabel*headerLabel;
    QLabel*registerTitle;

    // 账户密码
    QLineEdit *accountEdit;
    QLineEdit *passwordEdit;
    QLineEdit *passwordSure;
    QAction *labelForPwd;
    QAction *labelForPwdSure;

    // 邮箱
    QLineEdit *emailEdit;

    // 获取验证码
    QPushButton *getSecurityCode;
    QLineEdit *securityCode;

    // 注册按钮,取消
    QPushButton *registerBtn;
    QPushButton *cancelBtn;

};

#endif // REGISTERSCREEN_H
