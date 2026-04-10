#include "registerscreen.h"
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QRegularExpression>
#include <QTimer>
#include <QMessageBox>
#include "../stylemanager.h"
#include "../httpmanager.h"

RegisterScreen::RegisterScreen(QWidget *parent)
    : QWidget{parent}
{
    initHttpHandlers();
    setupUI();
    setupConnections();

    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    setFocusPolicy(Qt::ClickFocus);   // 允许鼠标点击时自己拿焦点

    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.width() - width()/2,screenGeometry.height() - height()/2);
}

void RegisterScreen::setupUI()
{
    this->setObjectName("registerScreen");
    this->setAttribute(Qt::WA_StyledBackground);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,30,30,30);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    // 表单布局
    formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignHCenter);

    // 头像
    headerLabel = new QLabel;
    headerLabel->setObjectName("headerLabel");
    headerLabel->setFixedSize(80,80);
    headerLabel->setAlignment(Qt::AlignHCenter);
    headerLabel->setPixmap(QPixmap(":/Resources/QQ.png"));
    headerLabel->setScaledContents(true);

    registerTitle = new QLabel;
    registerTitle->setObjectName("registerTitle");
    registerTitle->setText("注册账号");
    registerTitle->setProperty("state","normal");

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addStretch();
    headerLayout->addWidget(headerLabel);
    headerLayout->addWidget(registerTitle);
    headerLayout->addStretch();

    // 账号
    accountEdit = new QLineEdit();
    accountEdit->setObjectName("accountEdit");
    accountEdit->setFixedHeight(30);
    accountEdit->setAlignment(Qt::AlignHCenter);
    accountEdit->setPlaceholderText("输入用户名");
    accountEdit->clearFocus();
    formLayout->addRow("用户名:",accountEdit);

    // 邮箱
    emailEdit = new QLineEdit();
    emailEdit->setObjectName("emailEdit");
    emailEdit->setFixedHeight(30);
    emailEdit->setAlignment(Qt::AlignHCenter);
    emailEdit->setPlaceholderText("请输入邮箱");
    emailEdit->clearFocus();
    formLayout->addRow("邮箱:",emailEdit);

    // 密码
    passwordEdit = new QLineEdit();
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setFixedHeight(30);
    passwordEdit->setAlignment(Qt::AlignHCenter);
    passwordEdit->setPlaceholderText("输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setDragEnabled(false);
    labelForPwd = new QAction;
    labelForPwd->setCheckable(true);
    labelForPwd->setObjectName("labelForPwd");
    labelForPwd->setIcon(QIcon(":/Resources/nosee.png"));
    passwordEdit->addAction(labelForPwd,QLineEdit::TrailingPosition);
    formLayout->addRow("密码:",passwordEdit);

    // 确认密码
    passwordSure = new QLineEdit();
    passwordSure->setObjectName("passwordSure");
    passwordSure->setFixedHeight(30);
    passwordSure->setAlignment(Qt::AlignHCenter);
    passwordSure->setPlaceholderText("确认输入密码");
    passwordSure->setEchoMode(QLineEdit::Password);
    passwordSure->setDragEnabled(false);
    labelForPwdSure = new QAction;
    labelForPwdSure->setCheckable(true);
    labelForPwdSure->setObjectName("labelForPwdSure");
    labelForPwdSure->setIcon(QIcon(":/Resources/nosee.png"));
    passwordSure->addAction(labelForPwdSure ,QLineEdit::TrailingPosition);
    formLayout->addRow("密码:",passwordSure);

    // 验证码
    securityCode = new QLineEdit();
    securityCode->setObjectName("securityCode");
    securityCode->setFixedHeight(30);
    securityCode->setAlignment(Qt::AlignHCenter);
    securityCode->setPlaceholderText("输入验证码");

    getSecurityCode = new QPushButton();
    getSecurityCode->setObjectName("getSecurityCode");
    getSecurityCode->setText("获取");

    QHBoxLayout *securityCodeLayout = new QHBoxLayout;
    securityCodeLayout->addWidget(securityCode);
    securityCodeLayout->addWidget(getSecurityCode);
    formLayout->addRow("获取验证码:",securityCodeLayout);

    // 注册,取消
    registerBtn = new QPushButton();
    registerBtn->setObjectName("registerBtn2");
    registerBtn->setText("注册");
    registerBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    registerBtn->setMinimumWidth(60);
    registerBtn->setMinimumHeight(30);

    cancelBtn = new QPushButton();
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setText("取消");
    cancelBtn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    cancelBtn->setMinimumWidth(60);
    cancelBtn->setMinimumHeight(30);

    QHBoxLayout *registerAndCancelLayout = new QHBoxLayout;
    registerAndCancelLayout->addWidget(registerBtn);
    registerAndCancelLayout->addWidget(cancelBtn);

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(registerAndCancelLayout);

}


void RegisterScreen::setupConnections()
{
    // 取消返回登陆节目
    connect(cancelBtn,&QPushButton::clicked,this,[=](){emit goLogin();});
    // 点击获取验证码按钮
    connect(getSecurityCode,&QPushButton::clicked,this,&RegisterScreen::do_get_code_clicked);
    // 点击注册按钮
    connect(registerBtn,&QPushButton::clicked,this,&RegisterScreen::do_register_clicked);

    // 验证码
    connect(HttpManager::GetInstance().get(),&HttpManager::on_get_code_finished,this,&RegisterScreen::do_get_code_finished);
    // 注册
    connect(HttpManager::GetInstance().get(),&HttpManager::on_register_finished,this,&RegisterScreen::do_register_finished);
    // 可见性
    connect(labelForPwd, &QAction::toggled, [this](bool checked) {
        if (checked){
            passwordEdit->setEchoMode(QLineEdit::Normal);
            labelForPwd->setIcon(QIcon(":/Resources/see.png"));
        }else{
            passwordEdit->setEchoMode(QLineEdit::Password);
            labelForPwd->setIcon(QIcon(":/Resources/nosee.png"));
        }
    });
    connect(labelForPwdSure, &QAction::toggled, [this](bool checked) {
        if (checked){
            passwordSure->setEchoMode(QLineEdit::Normal);
            labelForPwdSure->setIcon(QIcon(":/Resources/see.png"));
        }else{
            passwordSure->setEchoMode(QLineEdit::Password);
            labelForPwdSure->setIcon(QIcon(":/Resources/nosee.png"));
        }
    });
    // 切换界面
    connect(this,&RegisterScreen::readyGoLogin,this,[this](){
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("注册成功");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // 自动删除
        msgBox->setText("2秒之后将会自动跳转到登录界面");
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->show();  // 使用 show() 而不是 exec()

        QTimer::singleShot(2000, this, [this,msgBox]() {
            // 在这里写切换界面的代码
            msgBox->accept();
            emit goLogin();
        });
    });
}

void RegisterScreen::showTip(int code,const QString&str)
{
    if(!code){
        registerTitle->setText(str);
        registerTitle->setProperty("state","error");
        StyleManager::repolish(registerTitle);
    }else{
        registerTitle->setText(str);
        registerTitle->setProperty("state","normal");
        StyleManager::repolish(registerTitle);
    }
}

bool RegisterScreen::doVerify(bool includingSecurityCode)
{
    // 是否未填写
    bool allFilled = true;
    // 是否包括验证码?分别用户获取验证码和登陆两个按钮的逻辑
    std::size_t count= formLayout->rowCount() -1;
    for(int i = 0;i<=count;++i){
        QLayoutItem*item = formLayout->itemAt(i,QFormLayout::FieldRole);
        if(item&&item->widget()){
            QLineEdit*lineEdit = static_cast<QLineEdit*>(item->widget());
            if(lineEdit && lineEdit->text().trimmed().isEmpty()){
                lineEdit->setToolTip("此项不能为空");
                lineEdit->setStyleSheet("border: 1px solid red;");
                allFilled = false;
            }
            else{
                lineEdit->setToolTip("");
                lineEdit->setStyleSheet("");
            }
        }
    }

    // 邮箱的布局不同单独处理
    if(includingSecurityCode){
        if(securityCode && securityCode->text().trimmed().isEmpty()){
            securityCode->setToolTip("此项不能为空");
            securityCode->setStyleSheet("border: 1px solid red;");
            allFilled = false;
        }
        else{
            securityCode->setToolTip("");
            securityCode->setStyleSheet("");
        }
    }

    // 邮箱
    auto email = emailEdit->text();
    QRegularExpression reg(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool matched = reg.match(email).hasMatch();

    RegisterVarify rv;
    rv = RegisterVarify::SUCCESS;
    QString passwordStr = passwordEdit->text().trimmed();
    QString passwordSureStr = passwordSure->text().trimmed();


    if (!matched){
        rv = RegisterVarify::ERROR_EMAIL_INCORRECTFORMAT;
    }else if(passwordStr != passwordSureStr){
        rv = RegisterVarify::ERROR_PASSWORD_NOTSURE;
    }else if(passwordStr.size()<6 || passwordStr.size()>12){
        rv = RegisterVarify::ERROR_PASSWORD_LEN;
    }
    else if(!allFilled){
        rv = RegisterVarify::ERROR_CONTENT_INCOMPLETE;
    }

    bool ok = false;
    switch(rv)
    {
    case RegisterVarify::ERROR_EMAIL_INCORRECTFORMAT:
        showTip(0,"邮箱格式不正确");
        break;
    case RegisterVarify::ERROR_PASSWORD_NOTSURE:
        showTip(0,"两次输入密码不同");
        passwordEdit->setStyleSheet("border: 1px solid red;");
        passwordEdit->setToolTip("此项不能为空");
        passwordSure->setStyleSheet("border: 1px solid red;");
        passwordSure->setToolTip("此项不能为空");
        break;
    case RegisterVarify::ERROR_PASSWORD_LEN:
        showTip(0,"密码长度在6-12位");
        break;
    case RegisterVarify::ERROR_CONTENT_INCOMPLETE:
        showTip(0,"请填写完整内容");
        break;
    case RegisterVarify::ERROR_SECURITY_EMPTY:
        showTip(0,"验证码为空");
        break;
    case RegisterVarify::SUCCESS:
        showTip(1,"请注意查收");
        ok = true;
        break;
    }
    return ok;
}

void RegisterScreen::initHttpHandlers()
{
    _handlers[RequestType::ID_GET_VARIFY_CODE] = [this](const QJsonObject&obj){
        ErrorCodes error = static_cast<ErrorCodes>(obj["error"].toInt());
        if(error!=ErrorCodes::SUCCESS){
            showTip(0,"验证码获取失败");
            return;
        }
        auto success = obj["success"].toBool();
        auto message = obj["message"].toString();
    };
    _handlers[RequestType::ID_REG_USER] = [this](const QJsonObject&obj){
        int error = obj["error"].toInt();
        if(error !=static_cast<int>(ErrorCodes::SUCCESS)){
            showTip(0,tr("账号或邮箱已被占用"));
            return;
        }
        auto email = obj["email"].toString();
        showTip(1,tr("用户注册成功"));
        emit readyGoLogin();
    };
}

void RegisterScreen::do_get_code_clicked()
{
    auto res= doVerify();
    if (!res) return;

    QJsonObject json_obj;
    json_obj["email"] = emailEdit->text().trimmed();
    HttpManager::GetInstance()->PostHttp(QUrl(gate_url_prefix+"/getSecurityCode"),json_obj,RequestType::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);

    timer = new QTimer(this);
    countdown = 60;

    getSecurityCode->setEnabled(false);
    getSecurityCode->setText(QString("%1s").arg(countdown));

    // 获取按钮的文字变化
    QObject::connect(timer,&QTimer::timeout,this,&RegisterScreen::do_change_get_code_btn);
    timer->start(1000);
    // 60s之后恢复
    QTimer::singleShot(30000,this,[this](){
        if (timer) {
            timer->stop();
            timer->deleteLater();
            timer = nullptr;
        }
        getSecurityCode->setText("获取");
        getSecurityCode->setEnabled(true);
    });
}

void RegisterScreen::do_get_code_finished(RequestType requestType,const QString&res,ErrorCodes errorCode)
{
    if(errorCode!=ErrorCodes::SUCCESS){
        showTip(0,tr("网络请求错误"));
        return;
    }
    // 解析json字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isEmpty()){
        showTip(0,"接收数据异常为空,无法解析");
        return;
    }
    if(!jsonDoc.isObject()){
        showTip(0,"接收数据异常,无法解析");
        return;
    }
    // 调用对应的回调解析
    _handlers[requestType](jsonDoc.object());
    return;
}

void RegisterScreen::do_change_get_code_btn()
{
    if (countdown > 0) {
        countdown--;
        getSecurityCode->setText(QString("(%1s)").arg(countdown));
    }
}

void RegisterScreen::do_register_clicked()
{
    auto res= doVerify(true);
    if(!res){
        return;
    }

    QJsonObject j;
    j["user"] = accountEdit->text().trimmed();
    j["password"] = cryptoString(passwordEdit->text().trimmed());
    j["email"] = emailEdit->text().trimmed();
    j["securityCode"] = securityCode->text().trimmed();

    HttpManager::GetInstance()->PostHttp(QUrl(gate_url_prefix+"/userRegister"),
                                        j, RequestType::ID_REG_USER,Modules::REGISTERMOD);

}

void RegisterScreen::do_register_finished(RequestType requestType,const QString&res,ErrorCodes errorCode)
{
    if(errorCode!=ErrorCodes::SUCCESS){
        showTip(0,tr("网络请求错误"));
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isEmpty()){
        showTip(0,"接收数据异常为空,无法解析");
        return;
    }
    if(!jsonDoc.isObject()){
        showTip(0,"接收数据异常,无法解析");
        return;
    }

    _handlers[requestType](jsonDoc.object());
    return;
}
