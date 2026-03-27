#ifndef CHATTOPAREA_H
#define CHATTOPAREA_H

#include <QWidget>
#include <QLabel>
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QPen>
#include <QPainterPath>
#include <QPainter>
#include <QRadioButton>
#include "../../../Properties/global.h"
#include "SideNews/notificationpanel.h"

class QVBoxLayout;
class QTextEdit;
class QListWidgetItem;
class QToolButton;
class QHBoxLayout;
class QListWidget;
class QLineEdit;

class QPushButton;
class StatusLabel;
class StatusButton;
class AnimatedSearchBox;
class ClearAvatarLabel;
class FriendAddDialog;
class SideNews;
class ProfilePopup;
class ChatTopArea : public QWidget
{
    Q_OBJECT
  public:
    explicit ChatTopArea(QWidget *parent = nullptr);
    ~ChatTopArea();
    void setupUI();
    void setupConnections();
  signals:
    void on_search_friend(const QString&uid);
  public slots:
    void do_show_news();
    void do_show_red_dot();
    void do_unshow_red_dot();
    void do_profile_out();
    void do_edit_profile_out();

  private:
    // 变量声明顺序和ui顺序相同
    StatusLabel *statusLabel;
    AnimatedSearchBox *searchBox;
    ClearAvatarLabel *headerLabelFromChat;
    QLabel *redDot;
    QPushButton *newsBtn;
    QPushButton *foldBtn;

    FriendAddDialog *friendAddDialog;
    QList<std::shared_ptr<FriendInfo>>userLists;

    ProfilePopup *profilePopup;
    QTimer *hoverTimer;

    NotificationPanel *newsPanel;
    bool padding = false;   // 防止多次发送查询请求
  protected:
    void keyPressEvent(QKeyEvent *event)override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

class ProfilePopup: public QWidget
{
    Q_OBJECT
  private:
    // 控件
    QLabel *avatarLabel;
    QLabel *nameLabel;
    QLabel *genderLabel;     // 性别图标
    QLabel *userIdLabel;
    QLabel *statusLabel;
    QLabel *signatureLabel;
    QLabel *emailLabel;
    QPushButton *editButton;
    // 分隔线
    QFrame *separatorLine;

           // 设置信息的接口
    void setAvatar(const QString &avatar);
    void setName(const QString &name);
    void setGenderMale();    // 设置男性
    void setGenderFemale();  // 设置女性
    void setUserId(const QString &id);
    void setOnline(bool online);  // 设置在线状态
    void setSignature(const QString &signature);
    void setEmail(const QString &email);

    void setupUI();
    void setupConnections();
  signals:
    void on_profile_clicked();  // 编辑资料按钮点击信号
  public:
    explicit ProfilePopup(QWidget*parent = nullptr);
    void setUserInfo(std::shared_ptr<UserInfo>info);
  public slots:
    void do_init_profile();
    void do_edit_profile(std::shared_ptr<UserInfo>info);
    void do_change_profile_status(int status);
  protected:
    void paintEvent(QPaintEvent *event) override;
};


class EditProfileDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit EditProfileDialog(QWidget *parent = nullptr);
    explicit EditProfileDialog(QWidget *parent = nullptr,ProfilePopup*popup=nullptr);

           // 设置当前信息
    void setUserInfo(const QString &name,
                     const QString &email,
                     const QString &signature,
                     int sex,
                     const QString &avatar);

    void setUserInfo(std::shared_ptr<UserInfo>info);

           // 获取编辑后的信息
    QString getName() const;
    QString getEmail() const;
    QString getSignature() const;
    int getSex()const;
    QPixmap getAvatar() const;

    void setAvatar(const QString&)const;
    void setProfile(ProfilePopup*profile);

signals:
    void on_update_profile(std::shared_ptr<UserInfo>info);

  private slots:
    void do_select_avatar();
    void do_save_clicked();
    void do_cancel_clicked();

  private:
    void initUI();
    void setupConnections();

  private:
    // 控件
    QLabel *avatarLabel;
    QPushButton *changeAvatarBtn;

    QLabel *nameLabel;
    QLineEdit *nameEdit;

    QLabel *emailLabel;
    QLineEdit *emailEdit;

    QLabel *signatureLabel;
    QTextEdit *signatureEdit;

    QLabel *genderLabel;
    QRadioButton *maleRadio;
    QRadioButton *femaleRadio;
    QButtonGroup *genderGroup;

    QPushButton *saveButton;
    QPushButton *cancelButton;

    ProfilePopup *profilePopup;

           // 数据
    QPixmap currentAvatar;
    QString avatarPath;
};


class StatusLabel: public QLabel
{
    Q_OBJECT
  public:
    explicit StatusLabel(QWidget *parent = nullptr);

    void setStatus(const QString &status);
    void setStatus(int status);
    int getStatus();
    QString getStatusStr();
    void setDotColor(const QColor &color);
    void setEnabled(bool enabled = true);
    void setShowBorder(bool show)noexcept;
  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QString statusStr;
    int status;
    QColor dotColor;

    bool isHovered = false;
    bool isPressed = false;
    bool isEnabled = true;

    bool showBorder = true;
  signals:
    void clicked();
    void hover();

           // QWidget interface
  protected:
    void mousePressEvent(QMouseEvent *event)override;
    void enterEvent(QEnterEvent *event)override;
    void leaveEvent(QEvent *event)override;
    void mouseReleaseEvent(QMouseEvent *event)override;
};


class FriendsItem :public QWidget
{
    Q_OBJECT
  public:
    explicit FriendsItem(int uid,const QString&avatar_path = "",const QString&name = "",int sex = 1,int status = 0,bool isFriend = false,QWidget*parent=nullptr);
    void setupUI();
    void setupConnections();
    void setShowBorder(bool show)noexcept;
  signals:
    void on_apply_clicked(int uid);
  private:
    int _uid;
    QString _icon;
    QString _name;
    QPushButton *_applyFriend;
    StatusLabel *_statusLabel;
    QLabel * _avatar;
    int _sex;
    int _status;
    bool _isFriend;

};


class AnimatedSearchBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int searchWidth READ searchWidth WRITE setSearchWidth)

  public:
    explicit AnimatedSearchBox(QWidget *parent = nullptr);
    void toggleSearch();  // 切换搜索框显示/隐藏
    QString getContent();

  signals:
    void on_search_clicked(const QString &keyword);
  public slots:
    // 收缩查询框的按键
    void do_search_clcked();
    // 根据输入内容决定是否发送请求
    void do_text_changed(const QString &text);
    // 获取tcp回包，设置usersList
    void do_users_searched(QList<std::shared_ptr<FriendInfo>>)noexcept;

  private:
    // 发送tcp请求，查询用户
    void getSearchUsers(const QString &uid);
    int searchWidth() const { return textWidth; }
    void setSearchWidth(int width);
    void hideResults();
    void showResults();
    void addItemToResults();
    void updateResults();

    void setupUI();
    void startAnimation();
    void setupConnections();

  private:
    QHBoxLayout *layout;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPropertyAnimation *animation;
    QAction *clearAction;
    QGraphicsOpacityEffect *opacityEffect;
    QListWidget *resultList;
    QList<std::shared_ptr<FriendInfo>>usersList;

    int textWidth;
    bool isExpanded;
    // QObject interface
  public:
    bool eventFilter(QObject *watched, QEvent *event);

};



class ClearAvatarLabel : public QLabel {
    Q_OBJECT
public:
    explicit ClearAvatarLabel(QWidget* parent = nullptr);
public slots:
    void do_init_avatar();
    void do_update_avatar(const QString&avatar);
protected:
    void paintEvent(QPaintEvent* event) override;
};


class FriendAddDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit FriendAddDialog(QWidget *parent = nullptr);
    void searchFriend(int uid);
    void setUserName(const QString&name);
    void setUserUid(int uid);
    void setUserAvatar(const QString&avatar);
    void setRemark(const QString&remark);

  public slots:
    void do_add_friend(int uid);
  private:
    void setupUI();

           // UI components
    QLabel *titleLabel;
    QLabel *avatarLabel;
    QLabel *nameLabel;
    QLabel *uidLabel;
    QPushButton *addButton;
    QPushButton *cancelButton;
    QTextEdit *remarkEdit;

    int userUid;
    QString userName;
    QString userAvatar;
};




#endif // CHATTOPAREA_H
