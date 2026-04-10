#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include "Properties/singleton.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

class HttpManager : public QObject,public std::enable_shared_from_this<HttpManager>,public Singleton<HttpManager>
{
    Q_OBJECT
public:
    ~HttpManager();
private:
    friend class Singleton<HttpManager>;
    HttpManager();
public:
    void PostHttp(const QUrl&url,const QJsonObject&json,RequestType request_id,Modules mod);
private:
    QNetworkAccessManager _manager;
private slots:
    void do_http_finished(RequestType requestType,const QString&res,ErrorCodes errorCode,Modules mod);
signals:
    void on_http_finished(RequestType requestType,const QString&res,ErrorCodes errorCode,Modules mod);
    // 获取验证码
    void on_get_code_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
    // 注册
    void on_register_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
    // 忘记密码
    void on_forgot_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
    // 登陆
    void on_login_finished(RequestType requestType,const QString&res,ErrorCodes errorCode);
};

#endif // HTTPMANAGER_H
