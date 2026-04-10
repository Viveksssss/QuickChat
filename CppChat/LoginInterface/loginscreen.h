#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QObject>
#include <QWidget>
#include "../Properties/global.h"


class QComboBox;
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
class QStackedWidget;
class RegisterScreen;
class LoginScreen;
class ForgotScreen;

class AuthStack:public QWidget
{
    Q_OBJECT
public:
    explicit AuthStack(QWidget *parent = nullptr);
private:
    QStackedWidget*stackWidget;
    LoginScreen*loginScreen;
    RegisterScreen*registerScreen;
    ForgotScreen*forgotScreen;

    QLabel *closeBtn;
signals:
    void closeWindow();
    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};



class LoginScreen : public QWidget
{
    Q_OBJECT
public:
    explicit LoginScreen(QWidget *parent = nullptr);

signals:
    void goRegsiter();  // to AuthStack::
    void goForgotPassword();    // to AuthStack::
    void on_tcp_connect(ServerInfo si); // to TcpManager::do_tcp_connect

private:
    void setupUI();
    void setupStyles();
    void setupConnections();
    void initHandlers();

private slots:
    void do_login_clicked();
    void do_connect_success(bool success); // from TcpManager::on_connect_success
    void do_login_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
    void do_login_failed(int error); // from TcpManager::on_login_failed


private:
    QLabel*headerLabel;

    // 账户密码
    QLineEdit *accountEdit;
    QLineEdit *passwordEdit;

    // 同意协议
    QCheckBox *agreeCheck;

    // 记住密码,自动登陆
    QCheckBox *rememberPasswd;
    QCheckBox *autoLoginCheck;

    // 登陆按钮
    QPushButton *loginBtn;

    // 注册,忘记密码
    QPushButton *registerBtn;
    QPushButton *forgotLabel;

    // QWidget interface
    QHash<RequestType,std::function<void(const QJsonObject&)>> _handlers;


    int _uid;
    QString _token;

protected:
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent *event);
};

#endif // LOGINSCREEN_H
