#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QObject>


enum class Styles{
    BLUE,
    LIGHT,
    DARK
};


class StyleManager : public QObject
{
    Q_OBJECT
public:
    explicit StyleManager(QObject *parent = nullptr);
private:
    static bool changeStyleSheet(const QString&path);

public:
    static bool changeStyleSheet(Styles style);
    static void repolish(QWidget*);

signals:
};

#endif // STYLEMANAGER_H
