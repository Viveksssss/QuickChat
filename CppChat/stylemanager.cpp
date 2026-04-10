#include "stylemanager.h"
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QWidget>
#include <QStyle>



StyleManager::StyleManager(QObject *parent)
    : QObject{parent}
{}

bool StyleManager::changeStyleSheet(const QString &path)
{
    QFile file(path);
    if (!file.exists()) {
        qDebug() << "样式表文件不存在:" << path;
        return false;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
        qDebug() << "样式表加载成功:" << path;
        return true;
    } else {
        qDebug() << "无法打开样式表文件:" << path << "错误:" << file.errorString();
        return false;
    }
}

bool StyleManager::changeStyleSheet(Styles style)
{
    QString stylePath;
    switch(style){
    case Styles::BLUE:
        stylePath = ":/Styles/stylesheetBlue.qss";
        break;
    case Styles::LIGHT:
        stylePath = ":/Styles/stylesheetLight.qss";
        break;
    case Styles::DARK:
        stylePath = ":/Styles/stylesheetDark.qss";
        break;
    default:
        break;
    }
    return changeStyleSheet(stylePath);
}

void StyleManager::repolish(QWidget *w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
}
