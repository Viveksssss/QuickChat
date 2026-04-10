#include "toptitlepart.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

TopTitlePart::TopTitlePart(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
    setupConnections();
}

void TopTitlePart::setupUI()
{
    setContentsMargins(0,0,0,0);
    setFixedHeight(60);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(30,0,0,10);

    // logo
    logo = new QLabel;
    logo->setObjectName("logo");
    logo->setFixedSize(QSize(40,35));

    // logoTitle
    logoTitle = new QLabel;
    logoTitle->setObjectName("logoTitle");
    logoTitle->setText("QuickChat");
    QFont font = logoTitle->font();
    font.setWeight(QFont::Black);
    logoTitle->setFont(font);

    layout->addWidget(logo);
    layout->addWidget(logoTitle);
}

void TopTitlePart::setupConnections()
{

}
