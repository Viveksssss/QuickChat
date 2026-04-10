#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class AuthStack;
class MainScreen;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setupUI();
    void setConnections();

    void filesClean();
    void offLine();

    ~MainWindow();

signals:

private:
    AuthStack *stack;
    MainScreen *mainScreen;
    QTimer *_timer;

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // MAINWINDOW_H
