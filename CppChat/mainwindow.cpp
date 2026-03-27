#include "mainwindow.h"
#include "tcpmanager.h"
#include "usermanager.h"
#include "../database.h"
#include "LoginInterface/loginscreen.h"
#include "MainInterface/mainscreen.h"

#include <QDir>
#include <QGuiApplication>
#include <QJsonObject>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , _timer(new QTimer(this))
{
    setupUI();
    setConnections();
    stack = new AuthStack(this);
    DataBase::GetInstance().initialization();
    mainScreen = new MainScreen(this);
    setCentralWidget(stack);
    // QTimer::singleShot(50,[](){
    //     emit TcpManager::GetInstance()->on_switch_interface();
    // });
}

void MainWindow::setupUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::ClickFocus);   // 允许鼠标点击时自己拿焦点
    setFixedSize({300,400});
    setWindowIcon(QIcon(":/icons/Resources/aaaaaa.png"));
    setWindowTitle("QuickChat");
    setObjectName("mainWindow");

}

void MainWindow::setConnections()
{
    // 登陆界面跳转主页面
    connect(TcpManager::GetInstance().get(),&TcpManager::on_switch_interface,this,[this](){
        QWidget *old = centralWidget();
        if (old) {
            old->setParent(nullptr);  // 脱离 QMainWindow，避免被 delete
            old->hide();              // 可选：隐藏
        }
        setCentralWidget(mainScreen);
        mainScreen->init();
        mainScreen->show();
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();

        // 设置窗口大小为屏幕的 80%（留出边距）
        int width = screenGeometry.width() * 0.4;
        int height = screenGeometry.height() * 0.4;
        resize(width,height);

        setMinimumSize(720,500);
        setMaximumSize(1920,1080);
        // resize(width, height);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        // 或者如果希望固定大小：setFixedSize(width, height);

        // 居中显示
        move(screenGeometry.width()/2 - width/2,
             screenGeometry.height()/2 - height/2);

        _timer->start(10000);
    });
    // 登陆界面跳转登陆界面
    connect(TcpManager::GetInstance().get(),&TcpManager::on_switch_login,this,&MainWindow::offLine);

    connect(TcpManager::GetInstance().get(),&TcpManager::on_connection_closed,this,[this](){
        QMessageBox::information(this,"下线提示","心跳超时或网络异常，下线！");
        offLine();
    });

    connect(_timer,&QTimer::timeout,this,[this]{
        QJsonObject j;
        j["fromuid"] = UserManager::GetInstance()->GetUid();
        QJsonDocument doc(j);
        emit TcpManager::GetInstance()->on_send_data(RequestType::ID_HEART_BEAT_REQ,doc.toJson(QJsonDocument::Compact));
    });

    connect(TcpManager::GetInstance().get(),&TcpManager::on_no_connection,_timer,&QTimer::stop);
}

void MainWindow::filesClean()
{
    QDir tempDir(QDir::tempPath());
    QStringList filters;
    filters << "tmp_from_quick_chat_rounded_*";
    filters << "tmp_from_quick_chat_clipboard_*";
    filters << "tmp_from_quick_chat_aduio_*";
    filters << "tmp_from_quick_chat_video_*";
    filters << "tmp_from_quick_chat_image_*";
    filters << "audio_*";
    filters << "video_*";
    QFileInfoList files = tempDir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo &file : files) {
        QFile::remove(file.absoluteFilePath());
    }
}

void MainWindow::offLine()
{
    _timer->stop();
    QWidget *old = centralWidget();
    if (old) {
        old->setParent(nullptr);  // 脱离 QMainWindow，避免被 delete
        old->hide();              // 可选：隐藏
    }
    setCentralWidget(stack);
    stack->show();


    setupUI();
    setCentralWidget(stack);
    show();
    raise();
    activateWindow();
}

MainWindow::~MainWindow()
{
    filesClean();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角矩形背景
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);

    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    // 重要：调用基类的paintEvent
    QWidget::paintEvent(event);
}

