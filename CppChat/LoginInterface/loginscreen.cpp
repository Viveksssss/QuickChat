#include "loginscreen.h"
#include "forgotscreen.h"
#include "registerscreen.h"
#include "../Properties/global.h"
#include "../httpmanager.h"
#include "../tcpmanager.h"
#include "../usermanager.h"
#include "../Properties/global.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QStackedWidget>
#include <QRect>
#include <QToolTip>
#include <qevent.h>



LoginScreen::LoginScreen(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
    setupConnections();
    initHandlers();

    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    setFocusPolicy(Qt::ClickFocus);

    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.width() - width()/2,screenGeometry.height() - height()/2);
}

void LoginScreen::do_connect_success(bool success)
{
    if (success){
        showToolTip(loginBtn,"登陆成功");
        QJsonObject json;
        json["uid"] = _uid;
        json["token"] = _token;
        QJsonDocument doc(json);
        QByteArray str = doc.toJson(QJsonDocument::Compact);
        emit TcpManager::GetInstance()->on_send_data(RequestType::ID_CHAT_LOGIN,str);
    }else{
        showToolTip(loginBtn,"网络异常");
        loginBtn->setEnabled(true);
    }
}

void LoginScreen::setupUI()
{
    this->setObjectName("loginScreen");
    this->setAttribute(Qt::WA_StyledBackground);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,30,30,30);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    // 头像区域
    headerLabel = new QLabel(this);
    headerLabel->setObjectName("headerLabel");
    headerLabel->setFixedSize(80,80);
    headerLabel->setAlignment(Qt::AlignHCenter);
    headerLabel->setPixmap(QPixmap(":/Resources/QQ.png"));
    headerLabel->setScaledContents(true);

    // 账号
    accountEdit = new QLineEdit(this);
    accountEdit->setObjectName("accountEdit");
    accountEdit->setFixedHeight(42);
    accountEdit->setFixedWidth(240);
    accountEdit->setAlignment(Qt::AlignHCenter);
    accountEdit->setPlaceholderText("输入QuickChat账号");
    accountEdit->clearFocus();

    // 密码
    passwordEdit = new QLineEdit(this);
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setFixedHeight(42);
    passwordEdit->setFixedWidth(240);
    passwordEdit->setAlignment(Qt::AlignHCenter);
    passwordEdit->setPlaceholderText("输入QuickChat密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setDragEnabled(false);

    // 同意协议
    agreeCheck = new QCheckBox(this);
    agreeCheck->setObjectName("agreeCheck");
    agreeCheck->setCheckState(Qt::Unchecked);
    agreeCheck->setText("已阅读并同意服务协议和QuickChat隐私保护");
    agreeCheck->setMouseTracking(true);

    // 登陆按钮
    loginBtn = new QPushButton(this);
    loginBtn->setObjectName("loginBtn");
    loginBtn->setText("登陆");
    loginBtn->setFixedHeight(42);
    loginBtn->setFixedWidth(240);

    // 注册
    registerBtn = new QPushButton;
    registerBtn->setObjectName("registerBtn1");
    registerBtn->setText("注册账号");
    registerBtn->setCursor(Qt::PointingHandCursor);

    // 忘记密码
    forgotLabel = new QPushButton;
    forgotLabel->setObjectName("forgotLabel");
    forgotLabel->setText("忘记密码");
    forgotLabel->setCursor(Qt::PointingHandCursor);

    // 底层布局:注册,忘记密码
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(0);
    bottomLayout->setAlignment(Qt::AlignHCenter);
    bottomLayout->addStretch(2);
    bottomLayout->addWidget(registerBtn);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(forgotLabel);
    bottomLayout->addStretch(2);

    mainLayout->addStretch();
    mainLayout->addWidget(headerLabel,0,Qt::AlignHCenter);
    mainLayout->addWidget(accountEdit,0,Qt::AlignHCenter);
    mainLayout->addWidget(passwordEdit,0,Qt::AlignHCenter);
    mainLayout->addWidget(agreeCheck,0,Qt::AlignHCenter);
    mainLayout->addWidget(loginBtn,0,Qt::AlignHCenter);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);
}


void LoginScreen::setupConnections()
{
    // 跳转注册
    connect(registerBtn, &QPushButton::clicked, this, [=,this](){
        emit goRegsiter();
    });
    // 跳转忘记
    connect(forgotLabel, &QPushButton::clicked, this, [=,this](){
        emit goForgotPassword();
    });
    // 点击登陆
    connect(loginBtn,&QPushButton::clicked,this,&LoginScreen::do_login_clicked);
    // 登陆回包
    connect(HttpManager::GetInstance().get(),&HttpManager::on_login_finished,this,&LoginScreen::do_login_finished);
    // 连接服务器
    connect(this,&LoginScreen::on_tcp_connect,TcpManager::GetInstance().get(),&TcpManager::do_tcp_connect);
    // 连接服务器成功
    connect(TcpManager::GetInstance().get(),&TcpManager::on_connect_success,this,&LoginScreen::do_connect_success);
    // 登陆失败
    connect(TcpManager::GetInstance().get(),&TcpManager::on_login_failed,this,&LoginScreen::do_login_failed);
}

void LoginScreen::initHandlers()
{
    _handlers[RequestType::ID_LOGIN_USER] = [this](QJsonObject json){
        int error = json["error"].toInt();
        if(error != static_cast<int>(ErrorCodes::SUCCESS)){
            showToolTip(loginBtn,"参数错误");
            loginBtn->setEnabled(true);
            return;
        }
        showToolTip(loginBtn,"正在连接服务器");

        auto email = json["email"].toString();
        ServerInfo si;
        si.uid = json["uid"].toInt();
        si.host = json["host"].toString();
        si.port = json["port"].toString();
        si.token = json["token"].toString();
        si.email = json["email"].toString();
        si.name = json["name"].toString();

        _uid = si.uid;
        _token = si.token;

        emit on_tcp_connect(si);
    };
}

void LoginScreen::do_login_clicked()
{
    QString accountStr = accountEdit->text().trimmed();
    QString passwordStr = passwordEdit->text().trimmed();
    bool clicked = agreeCheck->isChecked();
    bool allFilled = true;
    // 账号框
    if (accountStr.isEmpty()){
        showToolTip(accountEdit,"此项不能为空");
        accountEdit->setStyleSheet("border: 1px solid red;");
        allFilled = false;
    }else{
        accountEdit->setStyleSheet("");
    }

    // 密码框
    if (passwordStr.isEmpty()){
        showToolTip(passwordEdit,"此项不能为空");
        passwordEdit->setStyleSheet("border: 1px solid red;");
        allFilled = false;
    }else if(passwordStr.size()<6 || passwordStr.size()>12){
        showToolTip(passwordEdit,"长度在6-12位");
        passwordEdit->setStyleSheet("border: 1px solid red;");
        allFilled = false;
    }else{
        passwordEdit->setStyleSheet("");
    }

    // 同意协议
    if (!clicked){
        showToolTip(agreeCheck,"请勾选同意协议");
        allFilled = false;
    }

    if (!allFilled)return;

    // 发送json
    QJsonObject json;
    json["user"] = accountStr;
    json["password"] = cryptoString(passwordStr);
    HttpManager::GetInstance()->PostHttp(QUrl(gate_url_prefix+"/userLogin"),json,RequestType::ID_LOGIN_USER,Modules::LOGINMOD);
    loginBtn->setEnabled(false);
}



void LoginScreen::do_login_finished(RequestType requestType,const QString&res,ErrorCodes errorCode)
{

    Defer defer([this]{
        loginBtn->setEnabled(true);
    });
    if (errorCode != ErrorCodes::SUCCESS){
        showToolTip(loginBtn,"网络请求错误");
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(res.toUtf8());
    if (doc.isNull()){
        showToolTip(loginBtn,"解析错误");
        return;
    }
    if (!doc.isObject()){
        showToolTip(loginBtn,"解析错误");
        return;
    }
    // _handlers[requestType](doc.object());
    // 安全检查：确保处理器存在
    auto it = _handlers.find(requestType);
    if (it == _handlers.end()) {
        showToolTip(loginBtn, "未知的请求类型");
        return;
    }
    it.value()(doc.object());
}

void LoginScreen::do_login_failed(int error)
{
    QString result = QString("登陆失败，Error %1").arg(error);
    showToolTip(loginBtn,result);
    loginBtn->setEnabled(true);
}

void LoginScreen::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this); // 初始化样式选项
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this); // 使用样式绘制
    // 如果需要，你还可以在这里添加自定义绘制代码
    QWidget::paintEvent(event); // 调用基类的paintEvent
}

void LoginScreen::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    clearFocus();
}

AuthStack::AuthStack(QWidget *parent)
    :QWidget(parent)
{
    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);

    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.width() - width()/2,screenGeometry.height() - height()/2);

    stackWidget = new QStackedWidget(this);
    loginScreen = new LoginScreen(this);
    registerScreen = new RegisterScreen(this);
    forgotScreen = new ForgotScreen(this);

    stackWidget->addWidget(loginScreen);
    stackWidget->addWidget(registerScreen);
    stackWidget->addWidget(forgotScreen);

    stackWidget->setCurrentIndex(0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(stackWidget);

    closeBtn = new QLabel(this);
    closeBtn->installEventFilter(this);
    closeBtn->setObjectName("closeBtn");
    closeBtn->setFixedSize(20,20);
    closeBtn->setGeometry(sizeHint().width()-70,15,15,15);

    connect(loginScreen,&LoginScreen::goRegsiter,this,[=,this](){
        stackWidget->setCurrentIndex(1);
    });
    connect(loginScreen,&LoginScreen::goForgotPassword,this,[=,this](){
        stackWidget->setCurrentIndex(2);
    });
    connect(registerScreen,&RegisterScreen::goLogin,this,[=,this](){
        stackWidget->setCurrentIndex(0);
    });
    connect(forgotScreen,&ForgotScreen::goLogin,this,[=,this](){
        stackWidget->setCurrentIndex(0);
    });
    connect(this,&AuthStack::closeWindow,this,[=,this](){
        qApp->closeAllWindows();
    });
}

bool AuthStack::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == closeBtn && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            emit closeWindow();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}
