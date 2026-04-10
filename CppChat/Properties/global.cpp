#include "global.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QToolTip>
#include <QWidget>


QString gate_url_prefix = "";


std::function<QString(QString)>cryptoString = [](QString input) -> QString{
    QByteArray data = input.toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return hash.toHex();
};


void showToolTip(QWidget *widget,const QString&str,int yOffset){
    QPoint gloPox = widget->mapToGlobal(QPoint(0,widget->height()+yOffset-20));
    QToolTip::showText(gloPox,str,widget,QRect(),3000);
}

Defer::Defer(std::function<void()> func)
    : m_func(func)
{
}
Defer::~Defer()
{
    m_func();
}
