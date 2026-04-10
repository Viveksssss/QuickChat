#ifndef TOPTITLEPART_H
#define TOPTITLEPART_H

/******************************************************************************
 *
 * @file       toptitlepart.h
 * @brief      顶部边栏
 *
 * @author     Vivek
 * @date       2025/10/30
 * @history
 *****************************************************************************/

#include <QObject>
#include <QWidget>


class QLabel;
class TopTitlePart : public QWidget
{
    Q_OBJECT
public:
    explicit TopTitlePart(QWidget *parent = nullptr);
private:
    void setupUI();
    void setupConnections();

signals:
private:
    QLabel *logo;
    QLabel *logoTitle;


    // QObject interface
public:
};

#endif // TOPTITLEPART_H
