#include "forgotscreen.h"
#include "../stylemanager.h"
#include "../httpmanager.h"
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QJsonObject>
#include <QTimer>
#include <QMessageBox>
ForgotScreen::ForgotScreen(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
    setupConnections();
    initHttpHandlers();

    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    setFocusPolicy(Qt::ClickFocus);   // 允许鼠标点击时自己拿焦点

    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.width() - width()/2,screenGeometry.height() - height()/2);
}

void ForgotScreen::setupUI()
{
    this->setObjectName("ForgotScreen");
    this->setAttribute(Qt::WA_StyledBackground);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    // 表单布局
    formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignHCenter);

    // 头像和标题
    headerLabel = new QLabel;
    headerLabel->setObjectName("headerLabel");
    headerLabel->setFixedSize(80, 80);
    headerLabel->setAlignment(Qt::AlignHCenter);
    headerLabel->setPixmap(QPixmap(":/Resources/QQ.png"));
    headerLabel->setScaledContents(true);

    labelForHeader = new QLabel;
    labelForHeader->setObjectName("labelForHeader");
    labelForHeader->setText("找回密码");
    labelForHeader->setProperty("state", "normal");

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addStretch();
    headerLayout->addWidget(headerLabel);
    headerLayout->addWidget(labelForHeader);
    headerLayout->addStretch();

    // 账号
    accountEdit = new QLineEdit();
    accountEdit->setObjectName("accountEdit");
    accountEdit->setFixedHeight(30);
    accountEdit->setAlignment(Qt::AlignHCenter);
    accountEdit->setPlaceholderText("输入用户名");
    accountEdit->clearFocus();
    formLayout->addRow("用户名:", accountEdit);

    // 邮箱
    emailEdit = new QLineEdit();
    emailEdit->setObjectName("emailEdit");
    emailEdit->setFixedHeight(30);
    emailEdit->setAlignment(Qt::AlignHCenter);
    emailEdit->setPlaceholderText("请输入注册邮箱");
    emailEdit->clearFocus();
    formLayout->addRow("邮箱:", emailEdit);

    // 验证码
    securityCode = new QLineEdit();
    securityCode->setObjectName("securityCode");
    securityCode->setFixedHeight(30);
    securityCode->setAlignment(Qt::AlignHCenter);
    securityCode->setPlaceholderText("输入验证码");

    getSecurityCode = new QPushButton();
    getSecurityCode->setObjectName("getSecurityCode");
    getSecurityCode->setText("获取验证码");
    getSecurityCode->setFixedSize(80, 30);

    QHBoxLayout *securityCodeLayout = new QHBoxLayout;
    securityCodeLayout->addWidget(securityCode);
    securityCodeLayout->addWidget(getSecurityCode);
    formLayout->addRow("验证码:", securityCodeLayout);

    // 新密码
    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setObjectName("newPasswordEdit");
    newPasswordEdit->setFixedHeight(30);
    newPasswordEdit->setAlignment(Qt::AlignHCenter);
    newPasswordEdit->setPlaceholderText("输入新密码");
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setDragEnabled(false);

    showNewPwdAction = new QAction;
    showNewPwdAction->setCheckable(true);
    showNewPwdAction->setObjectName("showNewPwdAction");
    showNewPwdAction->setIcon(QIcon(":/Resources/nosee.png"));
    newPasswordEdit->addAction(showNewPwdAction, QLineEdit::TrailingPosition);
    formLayout->addRow("新密码:", newPasswordEdit);

    // 确认新密码
    confirmNewPasswordEdit = new QLineEdit();
    confirmNewPasswordEdit->setObjectName("confirmNewPasswordEdit");
    confirmNewPasswordEdit->setFixedHeight(30);
    confirmNewPasswordEdit->setAlignment(Qt::AlignHCenter);
    confirmNewPasswordEdit->setPlaceholderText("确认新密码");
    confirmNewPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmNewPasswordEdit->setDragEnabled(false);

    showConfirmPwdAction = new QAction;
    showConfirmPwdAction->setCheckable(true);
    showConfirmPwdAction->setObjectName("showConfirmPwdAction");
    showConfirmPwdAction->setIcon(QIcon(":/Resources/nosee.png"));
    confirmNewPasswordEdit->addAction(showConfirmPwdAction, QLineEdit::TrailingPosition);
    formLayout->addRow("确认密码:", confirmNewPasswordEdit);

    // 确定和取消按钮
    acceptBtn = new QPushButton();
    acceptBtn->setObjectName("acceptBtn");
    acceptBtn->setText("确定");
    acceptBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    acceptBtn->setMinimumWidth(60);
    acceptBtn->setMinimumHeight(30);

    cancleBtn = new QPushButton();
    cancleBtn->setObjectName("cancleBtn");
    cancleBtn->setText("取消");
    cancleBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    cancleBtn->setMinimumWidth(60);
    cancleBtn->setMinimumHeight(30);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(acceptBtn);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(cancleBtn);
    buttonLayout->addStretch();

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

}


void ForgotScreen::setupConnections()
{
    // 验证码
    connect(getSecurityCode,&QPushButton::clicked,this,&ForgotScreen::do_get_code_clicked);
    connect(HttpManager::GetInstance().get(),&HttpManager::on_get_code_finished,this,&ForgotScreen::do_get_code_finished);
    // 注册
    connect(acceptBtn,&QPushButton::clicked,this,&ForgotScreen::do_forgot_clicked);
    connect(HttpManager::GetInstance().get(),&HttpManager::on_forgot_finished,this,&ForgotScreen::do_forgot_finished);
    // 取消按钮
    connect(cancleBtn,&QPushButton::clicked,this,[this](){
        emit goLogin();
    });
    // 可见性
    connect(showNewPwdAction, &QAction::toggled, this, [this](bool checked) {
        newPasswordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        showNewPwdAction->setIcon(QIcon(checked ? ":/Resources/see.png" : ":/Resources/nosee.png"));
    });
    connect(showConfirmPwdAction, &QAction::toggled, this, [this](bool checked) {
        QLineEdit *confirmEdit = findChild<QLineEdit*>("confirmNewPasswordEdit");
        if (confirmEdit) {
            confirmEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
            showConfirmPwdAction->setIcon(QIcon(checked ? ":/Resources/see.png" : ":/Resources/nosee.png"));
        }
    });
    connect(this,&ForgotScreen::readyGoLogin,this,[this](){
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("密码修改成功");
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


void ForgotScreen::showTip(int code,const QString&str)
{
    if(!code){
        labelForHeader->setText(str);
        labelForHeader->setProperty("state","error");
        StyleManager::repolish(labelForHeader);
    }else{
        labelForHeader->setText(str);
        labelForHeader->setProperty("state","normal");
        StyleManager::repolish(labelForHeader);
    }
}

bool ForgotScreen::doVerify(bool includingSecurityCode)
{
    // 是否未填写
    bool allFilled = true;
    // 是否包括验证码?分别用户获取验证码和登陆两个按钮的逻辑
    std::size_t count= (includingSecurityCode?20:1);
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


        // 邮箱
        auto email = emailEdit->text();
        QRegularExpression reg(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        bool matched = reg.match(email).hasMatch();

        RegisterVarify rv;
        rv = RegisterVarify::SUCCESS;
        QString passwordStr = newPasswordEdit->text().trimmed();
        QString passwordSureStr = confirmNewPasswordEdit->text().trimmed();


        if (!matched){
            rv = RegisterVarify::ERROR_EMAIL_INCORRECTFORMAT;
        }else if(securityCode->text().trimmed().isEmpty()){
            rv = RegisterVarify::ERROR_SECURITY_EMPTY;
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
        case RegisterVarify::ERROR_SECURITY_EMPTY:
            showTip(0,"验证码为空");
            break;
        case RegisterVarify::ERROR_PASSWORD_NOTSURE:
            showTip(0,"两次输入密码不同");
            newPasswordEdit->setStyleSheet("border: 1px solid red;");
            newPasswordEdit->setToolTip("此项不能为空");
            confirmNewPasswordEdit->setStyleSheet("border: 1px solid red;");
            confirmNewPasswordEdit->setToolTip("此项不能为空");
            break;
        case RegisterVarify::ERROR_PASSWORD_LEN:
            showTip(0,"密码长度在6-12位");
            break;
        case RegisterVarify::ERROR_CONTENT_INCOMPLETE:
            showTip(0,"请填写完整内容");
            break;
        case RegisterVarify::SUCCESS:
            showTip(1,"请注意查收");
            ok = true;
            break;
        }
        return ok;
    }else{
        return true;
    }
}



void ForgotScreen::initHttpHandlers()
{
    _handlers[RequestType::ID_GET_VARIFY_CODE] = [this](const QJsonObject&obj){
        ErrorCodes error = static_cast<ErrorCodes>(obj["error"].toInt());
        if(error!=ErrorCodes::SUCCESS){
            showTip(0,"验证码获取失败");
            return;
        }
    };

    _handlers[RequestType::ID_RESET_PWD] = [this](const QJsonObject&obj){
        int error = obj["error"].toInt();
        if(error !=static_cast<int>(ErrorCodes::SUCCESS)){
            showTip(0,tr("注册失败"));
            return;
        }
        auto email = obj["email"].toString();
        showTip(1,tr("密码修改成功"));
        emit readyGoLogin();
    };

}


void ForgotScreen::do_change_get_code_btn()
{
    if (countdown > 0) {
        countdown--;
        getSecurityCode->setText(QString("(%1s)").arg(countdown));
    }
}

void ForgotScreen::do_get_code_clicked()
{
    auto res= doVerify();
    if (!res) return;

    QJsonObject json_obj;
    json_obj["email"] = emailEdit->text().trimmed();
    HttpManager::GetInstance()->PostHttp(QUrl(gate_url_prefix+"/getSecurityCode"),json_obj,RequestType::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);

    timer = new QTimer(this);
    countdown = 30;

    getSecurityCode->setEnabled(false);
    getSecurityCode->setText(QString("%1s").arg(countdown));

    // 获取按钮的文字变化
    QObject::connect(timer,&QTimer::timeout,this,&ForgotScreen::do_change_get_code_btn);
    timer->start(1000);
    // 30s之后恢复
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

void ForgotScreen::do_get_code_finished(RequestType requestType,const QString&res,ErrorCodes errorCode)
{
    if (errorCode == ErrorCodes::ERROR_EMAIL_NOTFOUND){
        showTip(0,tr("尚未注册！"));
        return;
    }

    if(errorCode!=ErrorCodes::SUCCESS){
        showTip(0,tr("请求错误"));
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



void ForgotScreen::do_forgot_clicked()
{
    auto res= doVerify(true);
    if(!res){
        return;
    }

    QJsonObject j;
    j["uid"] = accountEdit->text().trimmed();
    j["email"] = emailEdit->text().trimmed();
    j["password"] = cryptoString(newPasswordEdit->text().trimmed());
    j["securityCode"] = securityCode->text().trimmed();

    HttpManager::GetInstance()->PostHttp(QUrl(gate_url_prefix+"/resetPassword"),
                                         j, RequestType::ID_RESET_PWD,Modules::FORGOTMOD);

}

void ForgotScreen::do_forgot_finished(RequestType requestType,const QString&res,ErrorCodes errorCode)
{
    if(errorCode!=ErrorCodes::SUCCESS){
        showTip(0,tr("网络请求错误"));
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isEmpty()){
        showTip(0,"接收数据异常,无法解析");
        return;
    }
    if(!jsonDoc.isObject()){
        showTip(0,"接收数据异常,无法解析");
        return;
    }

    _handlers[requestType](jsonDoc.object());
    return;
}
