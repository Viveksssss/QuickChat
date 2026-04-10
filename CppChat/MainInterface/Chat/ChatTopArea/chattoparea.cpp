#include "chattoparea.h"
#include "../../../usermanager.h"
#include "../../../tcpmanager.h"
#include "../../../Properties/imagecropperdialog.h"
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>
#include <QListWidget>
#include <QApplication>
#include <QTimer>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextEdit>
#include <QScrollBar>
#include <QApplication>
#include <QFileDialog>
#include <QButtonGroup>
#include <QBuffer>
#include "../../../Properties/signalrouter.h"

QString pixmapToBase64(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return QString();
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

           // 保存为 PNG 格式（可以改为 JPEG）
    pixmap.save(&buffer, "PNG");

           // 转换为 Base64
    QString base64 = QString::fromLatin1(byteArray.toBase64());

    return base64;
}

ChatTopArea::ChatTopArea(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
    setupConnections();
}

ChatTopArea::~ChatTopArea()= default;

void ChatTopArea::setupUI()
{
    friendAddDialog = new FriendAddDialog(this);

    QHBoxLayout *main_hlay = new QHBoxLayout(this);
    main_hlay->setContentsMargins(0,0,0,0);
    main_hlay->setAlignment(Qt::AlignRight);
    main_hlay->setSpacing(10);

    statusLabel = new StatusLabel(this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setStatus("在线");
    statusLabel->setFixedSize({60,30});

    searchBox = new AnimatedSearchBox;
    searchBox->setMaximumWidth(300);

    newsBtn = new QPushButton;
    newsBtn->setObjectName("newsBtn");
    newsBtn->setIcon(QIcon(":/Resources/main/news.png"));
    newsBtn->setIconSize({20,20});
    newsBtn->setFixedSize({30,30});

    redDot = new QLabel(newsBtn);
    redDot->setVisible(false);
    redDot->setFixedSize(10, 10);
    redDot->setStyleSheet("background: red; border-radius: 5px;");
    redDot->move(newsBtn->width() - 10, 2); // 右上角位置
    redDot->setAttribute(Qt::WA_TransparentForMouseEvents); // 穿透鼠标事

    headerLabelFromChat = new ClearAvatarLabel;
    headerLabelFromChat->setObjectName("headerLabelFromChat");
    headerLabelFromChat->setFixedSize({30,30});
    QPixmap pixmap(":/Resources/main/header-default.png");
    headerLabelFromChat->setPixmap(pixmap);
    headerLabelFromChat->setScaledContents(true);

    foldBtn = new QPushButton;
    foldBtn->setObjectName("foldBtn");
    foldBtn->setIcon(QIcon(":/Resources/main/unfold.png"));
    foldBtn->setIconSize({20,20});
    foldBtn->setFixedSize({30,30});

    main_hlay->addWidget(statusLabel);
    main_hlay->addWidget(searchBox);
    main_hlay->addWidget(newsBtn);
    main_hlay->addWidget(headerLabelFromChat);
    main_hlay->addWidget(foldBtn);

    newsPanel = new NotificationPanel(this);
    newsPanel->setObjectName("NotificationPanel");
    newsPanel->setFixedWidth(250);
    newsPanel->hide();

    hoverTimer = new QTimer(this);
    hoverTimer->setSingleShot(true);
    hoverTimer->setInterval(500);
    profilePopup = new ProfilePopup(this);
    profilePopup->setUserInfo(UserManager::GetInstance()->GetUserInfo());


    qApp->installEventFilter(this);
}

void ChatTopArea::setupConnections()
{

    connect(this,&ChatTopArea::on_search_friend,searchBox,&AnimatedSearchBox::do_text_changed);

    connect(newsBtn,&QPushButton::clicked,this,&ChatTopArea::do_show_news);

    connect(newsPanel,&NotificationPanel::on_unshow_red_dot,this,&ChatTopArea::do_unshow_red_dot);

    connect(newsPanel,&NotificationPanel::on_show_red_dot,this,&ChatTopArea::do_show_red_dot);

    connect(hoverTimer,&QTimer::timeout,this,&ChatTopArea::do_profile_out);

    connect(profilePopup,&ProfilePopup::on_profile_clicked,this,&ChatTopArea::do_edit_profile_out);


           // 在按钮点击的槽函数中
    connect(statusLabel, &StatusLabel::clicked, this, [this]() {
        // 创建菜单
        QMenu *menu = new QMenu(this);
        menu->setWindowFlags(Qt::Popup);  // 设置为弹出窗口
        menu->setStyleSheet(
            "QMenu {"
            "    background-color: white;"
            "    border: 1px solid #e0e0e0;"
            "    border-radius: 6px;"
            "    padding: 4px;"
            "}"
            "QMenu::item {"
            "    padding: 6px 12px;"
            "    border-radius: 4px;"
            "    margin: 2px;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #e3f2fd;"
            "}"
            );

               // 添加状态选项
        QAction *onlineAction = menu->addAction("🟢 在线");
        QAction *busyAction = menu->addAction("🔴 忙碌");
        QAction *offlineAction = menu->addAction("⚫ 离线");

               // 连接菜单项点击信号
        connect(onlineAction, &QAction::triggered, this, [this]() {
            statusLabel->setStatus("在线");
        });
        connect(busyAction, &QAction::triggered, this, [this]() {
            statusLabel->setStatus("忙碌");
        });
        connect(offlineAction, &QAction::triggered, this, [this]() {
            statusLabel->setStatus("离线");
        });

               // 在按钮下方显示菜单
        QPoint pos = statusLabel->mapToGlobal(QPoint(0, statusLabel->height()));
        menu->exec(pos);

               // 菜单关闭后自动删除
        menu->setAttribute(Qt::WA_DeleteOnClose);
    });
}

void ChatTopArea::do_show_news()
{
    newsPanel->showPanel();
}

void ChatTopArea::do_show_red_dot()
{
    redDot->setVisible(true);
}

void ChatTopArea::do_unshow_red_dot()
{
    redDot->setVisible(false);
}

void ChatTopArea::do_profile_out()
{
    if (!profilePopup)return;

    auto*parent = this->window();
    if (parent){
        parent->activateWindow();
        parent->raise();
        QApplication::processEvents();
    }

    QPoint globalPos = headerLabelFromChat->mapToGlobal(QPoint(0,0));
    int popupX = globalPos.x() + (headerLabelFromChat->pixmap().width() - profilePopup->width());
    int popupY = globalPos.y() + (headerLabelFromChat->height() + 5);

           // 防止超出屏幕
    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    if (popupX + profilePopup->width() > screen.right()) {
        popupX = screen.right() - profilePopup->width();
    }
    if (popupY + profilePopup->height() > screen.bottom()) {
        popupY = globalPos.y() - profilePopup->height() - 5; // 显示在上方
    }

    profilePopup->move(popupX,popupY);
    profilePopup->show();
    profilePopup->raise();
}

void ChatTopArea::do_edit_profile_out()
{
    std::shared_ptr<EditProfileDialog> dialog = std::make_shared<EditProfileDialog>(this,profilePopup);
    dialog->setUserInfo(UserManager::GetInstance()->GetUserInfo());
    dialog->exec();
}

void ChatTopArea::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        if(!padding){
            padding = true;
            QTimer::singleShot(1000,[this]()mutable{
                this->padding = false;
            });
            emit on_search_friend(this->searchBox->getContent());
        }
        return;
    }
    else{
        QWidget::keyPressEvent(event);
    }
}

bool ChatTopArea::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == headerLabelFromChat){
        switch(event->type()){
        case QEvent::Enter:
            hoverTimer->start();
            break;
        case QEvent::Leave:
            hoverTimer->stop();
            break;
        case QEvent::MouseButtonPress:
            if (profilePopup && profilePopup->isVisible()){
                profilePopup->hide();
            }
            hoverTimer->stop();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);;
}


StatusLabel::StatusLabel(QWidget *parent)
{}

void StatusLabel::setStatus(const QString &status)
{
    this->statusStr = status;
    // 根据状态设置圆点颜色
    if (this->statusStr == "在线"){
        dotColor = QColor(0x58f376);
        this->status = 1;
    }
    else if (this->statusStr == "忙碌"){
        dotColor = QColor(0xe90739);
        this->status = 2;
    }
    else if (this->statusStr == "离线"){
        dotColor = Qt::gray;
        this->status = 0;
    }
    UserManager::GetInstance()->SetStatus(this->status);
    emit SignalRouter::GetInstance().on_change_profile_status(this->status);
    update(); // 触发重绘
}

void StatusLabel::setStatus(int status)
{
    this->status = status;
    if (this->status == 1){
        dotColor = QColor(0x58f376);
        this->statusStr = "在线";
    }
    else if (this->status == 2){
        dotColor = QColor(0xe90739);
        this->statusStr = "忙碌";
    }
    else if (this->status == 0){
        dotColor = Qt::gray;
        this->statusStr = "离线";
    }
    UserManager::GetInstance()->SetStatus(status);
    emit SignalRouter::GetInstance().on_change_profile_status(this->status);
    update(); // 触发重绘
}

int StatusLabel::getStatus()
{
    return status;
}

QString StatusLabel::getStatusStr()
{
    return statusStr;
}

void StatusLabel::setDotColor(const QColor &color)
{
    dotColor = color;
    update();
}

void StatusLabel::setEnabled(bool enabled)
{
    isEnabled = enabled;
}

void StatusLabel::setShowBorder(bool show) noexcept
{
    this->showBorder = show;
}

void StatusLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();

    QColor color = Qt::white;
    if(isPressed){
        color = QColor("#f7f8e7");
    }else if (isHovered){
        color = QColor("#f4d4f7");
    }

           // 1. 圆角矩形
    if (showBorder){
        painter.setPen(QPen(dotColor,3));
        painter.setBrush(QBrush(color));
        painter.drawRoundedRect(rect.adjusted(2,2,-2,-2),10,10);
    }

           // 2. 绘制左侧圆点
    int dotSize = 8;
    int dotMargin = 8;
    QRect dotRect(dotMargin,
                  (rect.height() - dotSize) / 2,
                  dotSize, dotSize);

    painter.setPen(Qt::NoPen);
    painter.setBrush(dotColor);
    painter.drawEllipse(dotRect);

           // 3. 绘制右侧文字
    painter.setPen(Qt::black); // 黑色文字
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(false);
    painter.setFont(font);

    QRect textRect(dotRect.right() + 6, 0,
                   rect.width() - dotRect.right() - 12, rect.height());
    painter.drawText(textRect, Qt::AlignCenter, statusStr);
}

void StatusLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isEnabled){
        isPressed = true;
        emit clicked();
        update();
        event->accept();
    }else{
        QLabel::mousePressEvent(event);
    }
}

void StatusLabel::enterEvent(QEnterEvent *event)
{
    if (!isPressed && isEnabled){
        isHovered = true;
        update();
        emit hover();
    }
    QLabel::enterEvent(event);
}

void StatusLabel::leaveEvent(QEvent *event)
{
    if (isEnabled){
        isHovered = false;
        isPressed = false;
        update();
    }

    QLabel::leaveEvent(event);
}

void StatusLabel::mouseReleaseEvent(QMouseEvent *event)
{

    if (isEnabled){
        isPressed = false;
        update(); // 触发重绘恢复正常状态
        event->accept();
    }
}


AnimatedSearchBox::AnimatedSearchBox(QWidget *parent)
    : isExpanded(false)
{
    setupUI();
    setupConnections();
    qApp->installEventFilter(this);   // 监听整个程序
}

void AnimatedSearchBox::setupUI()
{
    setMinimumWidth(30);
    setMaximumWidth(200);
    setFixedHeight(30);
    setAttribute(Qt::WA_TranslucentBackground);


    QVBoxLayout *main_vlay = new QVBoxLayout(this);
    main_vlay->setContentsMargins(0,0,0,0);
    main_vlay->setSpacing(5);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setAlignment(Qt::AlignRight);
    searchLayout->setContentsMargins(0,0,0,0);

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Enter Name or ID");
    searchEdit->setFixedHeight(30);
    searchEdit->setObjectName("searchEdit");
    searchEdit->setMaxLength(25);
    searchEdit->hide(); // 默认隐藏
    clearAction = new QAction;
    clearAction->setObjectName("clearAction");
    clearAction->setIcon(QIcon(":/Resources/main/clear.png"));

    searchEdit->addAction(clearAction,QLineEdit::TrailingPosition);
    connect(clearAction,&QAction::triggered,this,[this]{
        searchEdit->clear();
    });

    searchButton = new QPushButton(this);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/Resources/main/add.png"));
    searchButton->setIconSize({20,20});
    searchButton->setFixedSize({30,30});
    searchButton->setToolTip("Search");


    resultList = new QListWidget(window());
    resultList->setObjectName("resultList");
    resultList->setFixedSize(380, 320);  // 使用 setFixedSize
    resultList->setUniformItemSizes(true);
    resultList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    resultList->setVerticalScrollMode(QListWidget::ScrollPerItem); // 对滚轮无效。。。
    resultList->viewport()->installEventFilter(this); // 手写过滤器，每次滚动一项
    resultList->hide();
    resultList->setSpacing(1);
    QTimer::singleShot(0, this, [this] {
        QWidget *central = window();           // 普通 QWidget 场景
        resultList->setParent(central);
        resultList->setWindowFlags(Qt::Popup);         // 变回普通子控件
    });

    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);

    main_vlay->addLayout(searchLayout);

           // 动画
    animation = new QPropertyAnimation(this,"searchWidth");
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCurve);

    opacityEffect = new QGraphicsOpacityEffect(searchEdit);
    opacityEffect->setOpacity(0.0);
    searchEdit->setGraphicsEffect(opacityEffect);
}

void AnimatedSearchBox::toggleSearch()
{
    isExpanded = !isExpanded;
    if(!isExpanded){
        animation->setStartValue(150);
        animation->setEndValue(0);
        hideResults();
    }else{
        searchEdit->show();
        animation->setStartValue(0);
        animation->setEndValue(150);
    }
    startAnimation();
}

QString AnimatedSearchBox::getContent()
{
    return !searchEdit ? "" : searchEdit->text().trimmed();
}

void AnimatedSearchBox::do_search_clcked()
{
    toggleSearch();
}

void AnimatedSearchBox::do_text_changed(const QString &text)
{
    if (text.length() >= 1){
        getSearchUsers(text.trimmed());
        // updateResults();
        // showResults();
    }else{
        hideResults();
    }
}

void AnimatedSearchBox::do_users_searched(QList<std::shared_ptr<FriendInfo>>list)noexcept
{
    this->usersList = std::move(list);
    updateResults();
    showResults();
}


void AnimatedSearchBox::setSearchWidth(int width)
{
    textWidth = width;
    searchEdit->setFixedWidth(width);
}

void AnimatedSearchBox::hideResults()
{
    resultList->hide();
    resultList->clear();
}

void AnimatedSearchBox::showResults()
{
    if (resultList->count() == 0) {
        return;
    }

    if (!resultList->parent()) {
        resultList->setParent(window());
    }

    QRect r = searchEdit->rect();
    QPoint bottomLeft = searchEdit->mapToGlobal(r.bottomLeft());
    bottomLeft.setX(bottomLeft.x()-80);
    bottomLeft.setY(bottomLeft.y()+5);


    resultList->move(bottomLeft);
    resultList->show();
    resultList->raise();

           // 强制更新
           // resultList->update();
           // resultList->repaint();
}

void AnimatedSearchBox::updateResults(){
    resultList->clear();
    for (const std::shared_ptr<FriendInfo> &user : this->usersList) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(QSize(350,42));
        FriendsItem *friendItem = new FriendsItem(user->id,user->avatar,user->name,user->sex,user->status,user->isFriend);
        resultList->addItem(item);
        resultList->setItemWidget(item,friendItem);
    }

    if (resultList->count() == 0) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(QSize(350,42));
        // 提取用户ID - 实际项目中从数据结构获取
        item->setText("未查询到用户");
        item->setTextAlignment(Qt::AlignCenter);
        resultList->addItem(item);
    }

}

void AnimatedSearchBox::getSearchUsers(const QString &uid)
{
    QJsonObject obj;
    obj["fromUid"] = UserManager::GetInstance()->GetUid();
    obj["toUid"] = uid.trimmed();

    QJsonDocument doc(obj);

    emit TcpManager::GetInstance()->on_send_data(RequestType::ID_SEARCH_USER_REQ,doc.toJson(QJsonDocument::Compact));
}

void AnimatedSearchBox::startAnimation()
{
    animation->start();

    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnimation->setDuration(300);
    opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);

    if (isExpanded) {
        opacityAnimation->setStartValue(0.0);
        opacityAnimation->setEndValue(1.0);
        searchEdit->setFocus();
        clearAction->setVisible(true);
        showResults();
    } else {
        opacityAnimation->setStartValue(1.0);
        opacityAnimation->setEndValue(0.0);
        connect(opacityAnimation, &QPropertyAnimation::finished, [this]() {
            searchEdit->hide();
            searchEdit->clear();
            clearAction->setVisible(false);
        });
    }

    opacityAnimation->start(QPropertyAnimation::DeleteWhenStopped);
}

void AnimatedSearchBox::setupConnections()
{
    connect(searchButton,&QPushButton::clicked,this,&AnimatedSearchBox::do_search_clcked);

    connect(searchEdit,&QLineEdit::returnPressed,[this](){
        emit on_search_clicked(searchEdit->text());
    });

    connect(TcpManager::GetInstance().get(),&TcpManager::on_users_searched,this,&AnimatedSearchBox::do_users_searched);
}

bool AnimatedSearchBox::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && resultList->isVisible()) {
        QWidget *clickedWidget = qobject_cast<QWidget*>(obj);
        if (!clickedWidget) {
            return QWidget::eventFilter(obj, event);
        }

               // 获取全局鼠标位置
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPosition().toPoint();

               // 检查点击是否在resultList区域内
        QRect resultListRect = resultList->geometry();
        if (resultListRect.contains(globalPos)) {
            return QWidget::eventFilter(obj, event);
        }

               // 检查点击是否在searchEdit区域内
               // QRect searchEditRect = searchEdit->geometry();
               // searchEditRect.moveTopLeft(searchEdit->mapToGlobal(QPoint(0, 0)));
               // if (searchEditRect.contains(globalPos)) {
               //     return QWidget::eventFilter(obj, event);
               // }

               // 如果都不在，隐藏结果
        hideResults();
    }else if(event->type() == QEvent::Wheel && obj == resultList->viewport()){
        QWheelEvent*wheelEvent = static_cast<QWheelEvent*>(event);
        int delta = wheelEvent->angleDelta().y();
        if (delta == 0){
            return false;
        }

        if (delta > 0) {
            // 向上滚动一个项
            resultList->verticalScrollBar()->setValue(resultList->verticalScrollBar()->value() - 1);
        } else {
            // 向下滚动一个项
            resultList->verticalScrollBar()->setValue(resultList->verticalScrollBar()->value() + 1);
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}


FriendAddDialog::FriendAddDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void FriendAddDialog::searchFriend(int uid)
{
    QPixmap avatar(userAvatar);
    avatarLabel->setPixmap(avatar.scaled(50,50));
}

void FriendAddDialog::setupUI()
{
    // 默认头像
    userAvatar = ":/Resources/main/header.png";

           // 设置对话框属性 - 只调用一次
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setObjectName("friendAddDialog");
    setFixedSize(320, 320); // 增加高度以适应备注框
    // 移除了重复的 setWindowFlags

           // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5); // 减少间距
    mainLayout->setContentsMargins(10,0,10,10); // 调整边距

           // 标题
    titleLabel = new QLabel("添加好友");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

           // 用户信息区域（圆角框）
    QFrame *userInfoFrame = new QFrame();
    userInfoFrame->setObjectName("userInfoFrame");
    userInfoFrame->setFixedHeight(80);

    QHBoxLayout *userLayout = new QHBoxLayout(userInfoFrame);
    userLayout->setContentsMargins(15, 10, 15, 10);
    userLayout->setSpacing(15);

           // 头像 - 设置图片
    avatarLabel = new QLabel();
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setFixedSize(50, 50);
    QPixmap avatarPixmap(userAvatar);
    if (!avatarPixmap.isNull()) {
        avatarLabel->setPixmap(avatarPixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // 设置默认样式
        avatarLabel->setStyleSheet("background-color: #dee2e6; border-radius: 8px;");
    }

           // 姓名和UID区域 - 使用占位文本
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(5);
    infoLayout->setAlignment(Qt::AlignVCenter);

    nameLabel = new QLabel("用户名"); // 使用占位文本
    nameLabel->setObjectName("nameLabel");

    uidLabel = new QLabel("UID: "); // 使用占位文本
    uidLabel->setObjectName("uidLabel");

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(uidLabel);

    userLayout->addWidget(avatarLabel);
    userLayout->addLayout(infoLayout);
    userLayout->addStretch();

    mainLayout->addWidget(userInfoFrame);

           // 备注输入区域
    QLabel *remarkLabel = new QLabel("备注");
    remarkLabel->setObjectName("remarkLabel");
    mainLayout->addWidget(remarkLabel);

    remarkEdit = new QTextEdit();
    remarkEdit->setObjectName("remarkEdit");
    remarkEdit->setFixedHeight(60);
    remarkEdit->setPlaceholderText("请输入备注信息（可选）");
    mainLayout->addWidget(remarkEdit);

           // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10); // 减少按钮间距

    addButton = new QPushButton("添加");
    addButton->setObjectName("addButton");
    addButton->setFixedSize(80, 32);

    cancelButton = new QPushButton("取消");
    cancelButton->setObjectName("cancelButton");
    cancelButton->setFixedSize(80, 32);

    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

           // 连接信号槽
    connect(cancelButton, &QPushButton::clicked, this, &FriendAddDialog::reject);
    connect(addButton, &QPushButton::clicked, this, &FriendAddDialog::accept);
}
void FriendAddDialog::setUserName(const QString &name)
{
    this->userName = name;
}

void FriendAddDialog::setUserUid(int uid)
{
    this->userUid = uid;
}

void FriendAddDialog::setUserAvatar(const QString &avatar)
{
    this->userAvatar = avatar;
}

void FriendAddDialog::do_add_friend(int uid)
{
    setUserUid(uid);
    searchFriend(uid);
    exec();
}


FriendsItem::FriendsItem(int uid, const QString &avatar, const QString &name,int sex,int status,bool isFriend,QWidget*parent)
    : QWidget(parent)
    , _uid(uid)
    , _icon(avatar)
    , _name(name)
    , _status(status)
    , _sex(sex)
    , _isFriend(isFriend)
{
    setupUI();
    setupConnections();
}

void FriendsItem::setupUI()
{
    QHBoxLayout*main_hlay = new QHBoxLayout(this);
    main_hlay->setContentsMargins(10,0,10,0);
    main_hlay->setSpacing(5);

    _avatar = new QLabel;
    _avatar->setFixedSize(44, 44);  // 与finalPixmap尺寸一致
    _avatar->setAlignment(Qt::AlignCenter);  // 关键：内容居中

           // 使用原来的数据加载逻辑
    QPixmap originalPixmap;
    // 创建带边框的圆形图片
    if (_icon.startsWith(":/") || _icon.isEmpty()){
        originalPixmap = QPixmap(":/Resources/main/header-default.png");
    }else{
        QByteArray imageData = QByteArray::fromBase64(_icon.toLatin1());
        originalPixmap.loadFromData(imageData);
    }

           // 使用原来的绘制逻辑，改小尺寸
    QPixmap finalPixmap(36, 36);
    finalPixmap.fill(Qt::transparent);

    QPainter painter(&finalPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

           // 1. 先绘制边框
    QColor borderColor = (_sex == 1) ? QColor("#00F5FF") : QColor("#FF69B4");
    painter.setBrush(borderColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 36, 36);

           // 2. 绘制背景
    painter.setBrush(QColor("#E3F2FD"));
    painter.drawEllipse(2, 2, 32, 32);  // 边框内部

           // 3. 裁剪并绘制头像
    QPainterPath clipPath;
    clipPath.addEllipse(2, 2, 32, 32);  // 头像区域
    painter.setClipPath(clipPath);
    painter.drawPixmap(2, 2, 32, 32, originalPixmap.scaled(32, 32, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

           // 设置最终的头像图片
    _avatar->setPixmap(finalPixmap);
    _avatar->setAlignment(Qt::AlignCenter);  // 关键：内容居中
    _avatar->setScaledContents(true);   // 关键一行

    finalPixmap.save("/mnt/S/home/vivek/ppp.png");

    QLabel*name = new QLabel;
    name->setText(_name);
    QFont font = name->font();
    font.setBold(true);
    font.setPointSize(10);
    QPalette plt = name->palette();
    plt.setColor(QPalette::WindowText,QColor(333333));
    name->setFont(font);
    name->setPalette(plt);
    name->setAlignment(Qt::AlignCenter);

    _statusLabel = new StatusLabel;
    _statusLabel->setStatus(_status==0?"离线":"在线");
    _statusLabel->setEnabled(false);
    _statusLabel->setFixedSize({60,32});
    _statusLabel->setShowBorder(false);

    _applyFriend = new QPushButton;
    if (_uid == UserManager::GetInstance()->GetUid() || _isFriend){
        _applyFriend->setText("已添加");
        _applyFriend->setEnabled(false);    // 不允许自己添加自己。
    }else{
        _applyFriend->setText("添加");
    }
    _applyFriend->setFixedSize({60,30});
    _applyFriend->setStyleSheet(R"(
        QPushButton {
            background-color: #6bb9ef;

            color: #ffffff;
            border: none;
            border-radius: 10px;
            font-size: 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #3fd9d4;
        }
    )");

    main_hlay->addWidget(_avatar,Qt::AlignCenter);
    main_hlay->addSpacing(5);
    main_hlay->addWidget(name);
    main_hlay->addStretch();
    main_hlay->addWidget(_statusLabel);
    main_hlay->addWidget(_applyFriend);
}

void FriendsItem::setupConnections()
{
    connect(_applyFriend,&QPushButton::clicked,this,[this](bool){
        QJsonObject obj;
        obj["fromUid"] = static_cast<int>(UserManager::GetInstance()->GetUid());
        obj["fromName"] = UserManager::GetInstance()->GetName();
        obj["fromEmail"] = UserManager::GetInstance()->GetEmail();
        obj["fromDesc"] = UserManager::GetInstance()->GetDesc();
        obj["fromSex"] = UserManager::GetInstance()->GetSex();
        if (UserManager::GetInstance()->GetIcon().startsWith(":/")){
            obj["fromIcon"] = UserManager::GetInstance()->pixmapToBase64(UserManager::GetInstance()->GetIcon());
        }else{
            obj["fromIcon"] = UserManager::GetInstance()->GetIcon();
        }

        obj["toUid"] = this->_uid; // 对方的uid

        QJsonDocument doc;
        doc.setObject(obj);
        QByteArray data = doc.toJson(QJsonDocument::Compact);

        emit TcpManager::GetInstance()->on_send_data(RequestType::ID_ADD_FRIEND_REQ,data);
        this->_applyFriend->setEnabled(false);
        showToolTip(_applyFriend,"已发送好友请求");
    });
}

void FriendsItem::setShowBorder(bool show) noexcept
{
    _statusLabel->setShowBorder(show);
}

void ProfilePopup::setupUI()
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setFixedSize(320,330);

    setStyleSheet(R"(
        QLabel {
            background: transparent;
        }
        QPushButton {
            background: #07C160;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
        }
        QPushButton:hover {
            background: #06AD56;
        }
    )");


           // 头像标签（左侧）
    avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(70, 70);
    avatarLabel->setStyleSheet(R"(
        QLabel {
            border-radius: 35px;
            border: 2px solid #E0E0E0;
            background: #F5F5F5;
        }
    )");

           // 姓名标签
    nameLabel = new QLabel("未设置", this);
    QFont nameFont = nameLabel->font();
    nameFont.setPointSize(16);
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);

           // 性别标签（使用emoji或图标）
    genderLabel = new QLabel("♂",this);
    genderLabel->setFixedSize(24, 24);
    genderLabel->setStyleSheet("background: transparent;color: #666666; font-size: 12px;");

           // ID标签
    userIdLabel = new QLabel("ID: 100001", this);
    userIdLabel->setStyleSheet("color: #666666; font-size: 12px;");

           // 在线状态标签
    statusLabel = new QLabel("🟢 在线", this);
    statusLabel->setStyleSheet("color: #07C160; font-size: 12px;");

           // 分隔线
    separatorLine = new QFrame(this);
    separatorLine->setObjectName("line");
    separatorLine->setFixedHeight(1);
    separatorLine->setFrameShape(QFrame::HLine);

           // 个性签名标签
    signatureLabel = new QLabel("这个人很懒，什么都没有留下~", this);
    signatureLabel->setStyleSheet(R"(
        QLabel {
            color: #FF3366;
            font-size: 13px;
            padding: 5px 0px;
            border-radius:10px;
            border:1px solid #99FFFF;
        }
    )");
    signatureLabel->setWordWrap(true);
    signatureLabel->setMaximumWidth(280);
    signatureLabel->setMinimumHeight(100);

           // 邮箱标签
    QHBoxLayout *email_hlay = new QHBoxLayout;
    QLabel *email_pro = new QLabel("邮箱：");
    email_pro->setStyleSheet("color:#FF6600;");
    emailLabel = new QLabel("asdsa", this);
    emailLabel->setStyleSheet("color: #FF6666; font-size: 13px;");
    email_hlay->addWidget(email_pro);
    email_hlay->addWidget(emailLabel);
    email_hlay->addStretch();

           // 编辑资料按钮
    editButton = new QPushButton("编辑资料", this);
    editButton->setFixedHeight(36);

           // 顶部信息区域（头像+基本信息）
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(avatarLabel);
    topLayout->setSpacing(10);

           // 右侧信息垂直布局
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

           // 姓名和性别在同一行
    QHBoxLayout *nameGenderLayout = new QHBoxLayout();
    nameGenderLayout->addWidget(nameLabel);
    nameGenderLayout->addWidget(genderLabel);
    nameGenderLayout->addStretch();
    nameGenderLayout->setSpacing(5);
    nameGenderLayout->setContentsMargins(0, 0, 0, 0);

    infoLayout->addLayout(nameGenderLayout);
    infoLayout->addWidget(userIdLabel);
    infoLayout->addWidget(statusLabel);
    infoLayout->addStretch();

    topLayout->addLayout(infoLayout);
    topLayout->setContentsMargins(0, 0, 0, 0);

           // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(separatorLine);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(email_hlay);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(signatureLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(editButton, 0, Qt::AlignCenter);

    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(8);

}

void ProfilePopup::setupConnections()
{
    connect(editButton, &QPushButton::clicked, this, [this]() {
        emit on_profile_clicked();
        hide();  // 点击后隐藏弹窗
    });
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_init_profile,this,&ProfilePopup::do_init_profile);
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_change_profile_status,this,&ProfilePopup::do_change_profile_status);
}


ProfilePopup::ProfilePopup(QWidget *parent)
    : QWidget(nullptr)
{
    if (!parent->isActiveWindow()) {
        parent->activateWindow();
    }


    this->hide();
    setupUI();
    setupConnections();
}

void ProfilePopup::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::transparent);

           // 绘制圆角背景
    QPainterPath path;
    path.addRoundedRect(rect(), 30, 30);

    painter.fillPath(path, QColor(0xFF, 0xCC, 0xFF));  // #FF99CC

           // 绘制边框
    painter.setPen(QPen(QColor("#D1D1D1"), 1));
    painter.drawPath(path);

           // 绘制子控件
    QWidget::paintEvent(event);
}

// 设置信息的接口实现
void ProfilePopup::setAvatar(const QString &avatar)
{
    QPixmap pixmap;

    if (avatar.isEmpty() || avatar.startsWith(":/")) {
        pixmap = QPixmap(avatar);
    }
    else{
        QByteArray data = QByteArray::fromBase64(avatar.toUtf8());
        pixmap.loadFromData(data);
    }

           // 如果加载失败，使用默认头像
    if (pixmap.isNull()) {
        pixmap = QPixmap(":/Resources/main/header-default.png");
        if (pixmap.isNull()) {
            // 创建纯色圆形默认头像
            pixmap = QPixmap(70, 70);
            pixmap.fill(QColor("#07C160"));
        }
    }

           // 创建圆形头像
    QPixmap circular(70, 70);
    circular.fill(Qt::transparent);

    QPainter painter(&circular);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addEllipse(0, 0, 70, 70);
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, pixmap.scaled(70, 70, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    avatarLabel->setPixmap(circular);
}

void ProfilePopup::setName(const QString &name)
{
    nameLabel->setText(name);

           // 如果名字太长，添加省略号
    QFontMetrics metrics(nameLabel->font());
    QString elidedText = metrics.elidedText(name, Qt::ElideRight, 150);
    nameLabel->setText(elidedText);
    nameLabel->setToolTip(name.length() > elidedText.length() ? name : "");
}

void ProfilePopup::setGenderMale()
{
    genderLabel->setText("♂");
    genderLabel->setStyleSheet(R"(
        QLabel {
            color: #4A90E2;
            font-size: 16px;
            font-weight: bold;
            background: transparent;
        }
    )");
}

void ProfilePopup::setGenderFemale()
{
    genderLabel->setText("♀");
    genderLabel->setStyleSheet(R"(
        QLabel {
            color: #FF6B9D;
            font-size: 16px;
            font-weight: bold;
            background: transparent;
        }
    )");
}

void ProfilePopup::setUserId(const QString &id)
{
    userIdLabel->setText("ID: " + id);
}

void ProfilePopup::setOnline(bool online)
{
    if (online) {
        statusLabel->setText("🟢 在线");
        statusLabel->setStyleSheet("color: #07C160; font-size: 12px;");
    } else {
        statusLabel->setText("⚫ 离线");
        statusLabel->setStyleSheet("color: #999999; font-size: 12px;");
    }
}

void ProfilePopup::setSignature(const QString &signature)
{
    QString text = signature.isEmpty() ? "这个人很懒，什么都没有留下~" : signature;
    signatureLabel->setText("个性签名：" + text);

           // 设置tooltip显示完整签名
    if (text.length() > 30) {
        signatureLabel->setToolTip(text);
    }
}

void ProfilePopup::setEmail(const QString &email)
{
    QString text = email.isEmpty() ? "未设置" : email;
    emailLabel->setText(text);

    if (!email.isEmpty()) {
        emailLabel->setToolTip(email);
    }
}

void ProfilePopup::setUserInfo(std::shared_ptr<UserInfo> info)
{
    // uid
    userIdLabel->setText("ID: " + QString::number(info->id));

    // 头像
    setAvatar(info->avatar);

    // 名称
    nameLabel->setText(info->name);
    // 性别
    if (info->sex == 1){
        genderLabel->setText("♂");
    }else{
        genderLabel->setText("♀");
    }
    // 个性签名
    signatureLabel->setText(info->desc.isEmpty() ? "这个人很懒，什么都没有留下~" : info->desc);
    // 邮箱
    emailLabel->setText(info->email);
    // 状态
    if (info->status == 1) {
        statusLabel->setText("🟢 在线");
        statusLabel->setStyleSheet("color: #07C160; font-size: 12px;");
    } else if (info->status == 0) {
        statusLabel->setText("⚫ 离线");
        statusLabel->setStyleSheet("color: #999999; font-size: 12px;");
    }else if (info->status == 2){
        statusLabel->setText("🔴 忙碌");
        statusLabel->setStyleSheet("color: red; font-size: 12px;");
    }
}

void ProfilePopup::do_init_profile()
{
    setUserInfo(UserManager::GetInstance()->GetUserInfo());
}

void ProfilePopup::do_edit_profile(std::shared_ptr<UserInfo> info)
{
    setUserInfo(info);
    UserManager::GetInstance()->SetBaseInfo(info);
    QJsonObject obj;
    obj["uid"] = info->id;
    obj["email"] = info->email;
    obj["avatar"] = info->avatar;
    obj["sex"] = info->sex;
    obj["desc"] = info->desc;
    obj["name"] = info->name;
    QJsonDocument doc(obj);
    emit TcpManager::GetInstance()->on_send_data(RequestType::ID_SYNC_PERSONAL_INFORMATION_REQ,doc.toJson(QJsonDocument::Compact));
}

void ProfilePopup::do_change_profile_status(int status)
{
    // 状态
    if (status == 1) {
        statusLabel->setText("🟢 在线");
        statusLabel->setStyleSheet("color: #07C160; font-size: 12px;");
    } else if(status == 0){
        statusLabel->setText("⚫ 离线");
        statusLabel->setStyleSheet("color: #999999; font-size: 12px;");
    } else if (status == 2){
        statusLabel->setText("🔴 忙碌");
        statusLabel->setStyleSheet("color: red; font-size: 12px;");
    }
}


EditProfileDialog::EditProfileDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("编辑个人资料");
    setModal(true);  // 模态对话框
    setFixedSize(400, 430);  // 固定大小

    initUI();
    setupConnections();

           // 居中显示
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenRect = screen->availableGeometry();
        move(screenRect.center() - rect().center());
    }
}

EditProfileDialog::EditProfileDialog(QWidget *parent, ProfilePopup *popup)
    : QDialog(nullptr)
    , profilePopup(popup)
{
    setWindowTitle("编辑个人资料");
    setModal(true);  // 模态对话框
    setFixedSize(400, 430);  // 固定大小

    initUI();
    setupConnections();

           // 居中显示
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenRect = screen->availableGeometry();
        move(screenRect.center() - rect().center());
    }
}

void EditProfileDialog::initUI()
{
    // 设置对话框样式
    setStyleSheet(R"(
        QDialog {
            background: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 12px;
        }

        QLabel {
            color: #333333;
            font-size: 13px;
        }

        QLineEdit, QTextEdit {
            border: 1px solid #D1D1D1;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 13px;
            background: #FFFFFF;
            selection-background-color: #07C160;
        }

        QLineEdit:focus, QTextEdit:focus {
            border: 1px solid #07C160;
            outline: none;
        }

        QPushButton {
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: 500;
            min-width: 80px;
        }

        QPushButton#saveButton {
            background: #07C160;
            color: white;
        }

        QPushButton#saveButton:hover {
            background: #06AD56;
        }

        QPushButton#saveButton:pressed {
            background: #059C4D;
        }

        QPushButton#cancelButton {
            background: #F5F5F5;
            color: #666666;
        }

        QPushButton#cancelButton:hover {
            background: #E8E8E8;
        }

        QPushButton#changeAvatarBtn {
            background: #F5F5F5;
            color: #07C160;
            border: 1px dashed #D1D1D1;
            padding: 6px 12px;
            font-size: 12px;
        }

        QPushButton#changeAvatarBtn:hover {
            background: #E8F5E9;
            border-color: #07C160;
        }

        QRadioButton {
            spacing: 8px;
            font-size: 13px;
        }
    )");

           // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 20);
    mainLayout->setSpacing(20);

           // 头像区域
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->setSpacing(20);

    avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(100, 100);
    avatarLabel->setStyleSheet(R"(
        QLabel {
            border-radius: 50px;
            border: 3px solid #E0E0E0;
            background: #F5F5F5;
        }
    )");
    avatarLabel->setAlignment(Qt::AlignCenter);

    changeAvatarBtn = new QPushButton("更换头像", this);
    changeAvatarBtn->setObjectName("changeAvatarBtn");
    changeAvatarBtn->setFixedSize(100, 32);

    avatarLayout->addWidget(avatarLabel);

    QVBoxLayout *avatarRightLayout = new QVBoxLayout();
    avatarRightLayout->addStretch();
    avatarRightLayout->addWidget(changeAvatarBtn);
    avatarRightLayout->addStretch();

    avatarLayout->addLayout(avatarRightLayout);
    avatarLayout->addStretch();

    mainLayout->addLayout(avatarLayout);

           // 表单区域
    QGridLayout *formLayout = new QGridLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(0, 0, 0, 0);

           // 姓名
    nameLabel = new QLabel("姓名：", this);
    nameLabel->setFixedWidth(80);
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("请输入姓名");
    formLayout->addWidget(nameLabel, 0, 0);
    formLayout->addWidget(nameEdit, 0, 1);

           // 邮箱
    emailLabel = new QLabel("邮箱：", this);
    emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("请输入邮箱地址");
    formLayout->addWidget(emailLabel, 1, 0);
    formLayout->addWidget(emailEdit, 1, 1);

           // 性别
    genderLabel = new QLabel("性别：", this);

    QHBoxLayout *genderLayout = new QHBoxLayout();
    genderLayout->setSpacing(20);

    maleRadio = new QRadioButton("男性", this);
    femaleRadio = new QRadioButton("女性", this);
    maleRadio->setChecked(true);  // 默认男性

    genderGroup = new QButtonGroup(this);
    genderGroup->addButton(maleRadio);
    genderGroup->addButton(femaleRadio);

    genderLayout->addWidget(maleRadio);
    genderLayout->addWidget(femaleRadio);
    genderLayout->addStretch();

    formLayout->addWidget(genderLabel, 2, 0);
    formLayout->addLayout(genderLayout, 2, 1);

           // 个性签名
    signatureLabel = new QLabel("个性签名：", this);
    signatureEdit = new QTextEdit(this);
    signatureEdit->setPlaceholderText("请输入个性签名...");
    signatureEdit->setMaximumHeight(80);
    signatureEdit->setMinimumHeight(60);

    formLayout->addWidget(signatureLabel, 3, 0, Qt::AlignTop);
    formLayout->addWidget(signatureEdit, 3, 1);

           // 设置列拉伸
    formLayout->setColumnStretch(0, 0);  // 标签列不拉伸
    formLayout->setColumnStretch(1, 1);  // 输入框列拉伸

    mainLayout->addLayout(formLayout);

           // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    cancelButton = new QPushButton("取消", this);
    cancelButton->setObjectName("cancelButton");
    cancelButton->setFixedSize(100, 36);

    saveButton = new QPushButton("保存", this);
    saveButton->setObjectName("saveButton");
    saveButton->setFixedSize(100, 36);

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addSpacing(15);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void EditProfileDialog::setupConnections()
{
    connect(changeAvatarBtn, &QPushButton::clicked, this, &EditProfileDialog::do_select_avatar);
    connect(saveButton, &QPushButton::clicked, this, &EditProfileDialog::do_save_clicked);
    connect(cancelButton, &QPushButton::clicked, this, &EditProfileDialog::do_cancel_clicked);
    connect(this,&EditProfileDialog::on_update_profile,profilePopup,&ProfilePopup::do_edit_profile);
}

void EditProfileDialog::setUserInfo(const QString &name,
                                    const QString &email,
                                    const QString &signature,
                                    int sex,
                                    const QString &avatar)
{
    nameEdit->setText(name);
    emailEdit->setText(email);
    signatureEdit->setText(signature);

    if (sex == 1) {
        maleRadio->setChecked(true);
    } else {
        femaleRadio->setChecked(true);
    }
    setAvatar(avatar);
}

void EditProfileDialog::setUserInfo(std::shared_ptr<UserInfo> info)
{
    nameEdit->setText(info->name);
    emailEdit->setText(info->email);
    signatureEdit->setText(info->desc);
    if (info->sex == 1) {
        maleRadio->setChecked(true);
    } else {
        femaleRadio->setChecked(true);
    }
    setAvatar(info->avatar);
}

void EditProfileDialog::do_select_avatar()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "选择头像",
                                                    QDir::homePath(),
                                                    "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.webp)");
    QPixmap pixmap = ImageCropperDialog::getCroppedImage(fileName,600,400,CropperShape::CIRCLE,QSize());

    if (!fileName.isEmpty()) {
        if (!pixmap.isNull()) {
            avatarPath = fileName;
            currentAvatar = pixmap;

            // 创建圆形头像
            QPixmap circularAvatar = pixmap.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

            QPixmap rounded(100, 100);
            rounded.fill(Qt::transparent);

            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath path;
            path.addEllipse(0, 0, 100, 100);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, circularAvatar);

            avatarLabel->setPixmap(rounded);
        } else {
            QMessageBox::warning(this, "错误", "无法加载图片文件！");
        }
    }
}

void EditProfileDialog::do_save_clicked()
{

    auto info = std::make_shared<UserInfo>();
    QString name = nameEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();
    QString desc = signatureEdit->toPlainText().trimmed();
    int sex = this->maleRadio->isChecked();
    // 验证
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "姓名不能为空！");
        nameEdit->setFocus();
        return;
    }

    if (!email.isEmpty()) {
        // 简单的邮箱格式验证
        if (!email.contains('@') || !email.contains('.')) {
            QMessageBox::warning(this, "提示", "邮箱格式不正确！");
            emailEdit->setFocus();
            return;
        }
    }
    info->id = UserManager::GetInstance()->GetUid();
    info->name = name;
    info->email = email;
    info->desc = desc;
    info->sex = sex;
    info->avatar = pixmapToBase64(avatarLabel->pixmap());

    UserManager::GetInstance()->SetBaseInfo(info);

           // 保存成功，发射信号
    emit on_update_profile(info);
    emit SignalRouter::GetInstance().on_update_avatar(info->avatar);

    accept();  // 关闭对话框
}

void EditProfileDialog::do_cancel_clicked()
{
    reject();  // 关闭对话框
}

// 获取信息的函数
QString EditProfileDialog::getName() const
{
    return nameEdit->text().trimmed();
}

QString EditProfileDialog::getEmail() const
{
    return emailEdit->text().trimmed();
}

QString EditProfileDialog::getSignature() const
{
    return signatureEdit->toPlainText().trimmed();
}

int EditProfileDialog::getSex() const
{
    return maleRadio->isChecked() == 1 ;
}

QPixmap EditProfileDialog::getAvatar() const
{
    return currentAvatar;
}

void EditProfileDialog::setAvatar(const QString &avatar) const
{
    QPixmap pixmap;

    if (avatar.isEmpty() || avatar.startsWith(":/")) {
        pixmap = QPixmap(avatar);
    }
    else{
        QByteArray data = QByteArray::fromBase64(avatar.toUtf8());
        pixmap.loadFromData(data);
    }

           // 如果加载失败，使用默认头像
    if (pixmap.isNull()) {
        pixmap = QPixmap(":/Resources/main/header-default.png");
        if (pixmap.isNull()) {
            // 创建纯色圆形默认头像
            pixmap = QPixmap(100, 100);
            pixmap.fill(QColor("#07C160"));
        }
    }

           // 创建圆形头像
    QPixmap circular(100, 100);
    circular.fill(Qt::transparent);

    QPainter painter(&circular);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addEllipse(0, 0, 100, 100);
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, pixmap.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    avatarLabel->setPixmap(circular);
}

void EditProfileDialog::setProfile(ProfilePopup *profile)
{
    this->profilePopup = profile;
}


ClearAvatarLabel::ClearAvatarLabel(QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(30, 30);
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_update_avatar,this,&ClearAvatarLabel::do_update_avatar);
    connect(&SignalRouter::GetInstance(),&SignalRouter::on_init_avatar,this,&ClearAvatarLabel::do_init_avatar);
    this->installEventFilter(this);
}

void ClearAvatarLabel::do_init_avatar()
{
    do_update_avatar(UserManager::GetInstance()->GetIcon());
}

void ClearAvatarLabel::do_update_avatar(const QString &avatar)
{
    // 直接使用 toUtf8() 更安全
    QByteArray imageData = QByteArray::fromBase64(avatar.toUtf8());

    if (!imageData.isEmpty()) {
        QPixmap pix;
        if (pix.loadFromData(imageData)) {
            setPixmap(pix);
            update();
            return;
        }
    }

    // 如果失败，设置空
    setPixmap(QPixmap(":/Resources/main/header-default.png"));
    update();
}

void ClearAvatarLabel::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

           // 绘制头像
    if (!pixmap().isNull()) {
        // 获取高质量图片并缩放到合适尺寸
        // QPixmap originalPixmap = getHighQualityPixmap();
        QPixmap scaledPixmap = pixmap().scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

               // 圆形裁剪
        QPainterPath path;
        path.addEllipse(rect());
        painter.setClipPath(path);

               // 计算居中位置
        int x = (scaledPixmap.width() - width()) / 2;
        int y = (scaledPixmap.height() - height()) / 2;

               // 绘制图片（居中裁剪）
        painter.drawPixmap(rect(), scaledPixmap, QRect(x, y, width(), height()));
    }

           // 绘制边框
    painter.setClipping(false);
    painter.setPen(QPen(QColor("#3b3b3b"), 1));
    painter.drawEllipse(rect().adjusted(1, 1, -1, -1));
}
