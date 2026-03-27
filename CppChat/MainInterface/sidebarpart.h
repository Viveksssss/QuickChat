#ifndef SIDEBARPART_H
#define SIDEBARPART_H

/******************************************************************************
 *
 * @file       sidebarpart.h
 * @brief      左侧的边栏
 *
 * @author     Vivek
 * @date       2025/10/30
 * @history
 *****************************************************************************/


#include <QWidget>

class QButtonGroup;
class QVBoxLayout;
class QPushButton;
struct SideBarItem{
    QString id;   // 唯一标识
    QString text; // 文本
    QString icon; // 图标
    int order;    // 顺序
};


class SideBarPart : public QWidget
{
    Q_OBJECT
public:
    explicit SideBarPart(QWidget *parent = nullptr);
    void addItem(const QString&id,const QString&text,const QString&icon="");
    void removeItem(const QString&id);
private:
    void setupUI();
    void setupConnections();
    void createButton(const SideBarItem&item,bool showText = false);
signals:
    void on_sidebar_btn_clicked(const QString&); // to FriendsList or mainScreen
private slots:
    void do_eliminate_status();
private:
    QVBoxLayout *layout;
    QMap<QString,QPushButton*>buttons;
    QVector<SideBarItem>items;
    QButtonGroup*buttonGroup;

    int nextId = 0;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event)override;
};

#endif // SIDEBARPART_H
