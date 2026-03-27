#ifndef FRIENDITEM_H
#define FRIENDITEM_H


// FriendItem.h
#include <QString>
#include "../Properties/global.h"

struct FriendItem : public UserInfo {
    QString message;     // 最新的部分消息

    explicit FriendItem(int id,int status,int sex,const QString &name = "",
               const QString &avatar = "",const QString&desc = "",
               const QString &message = "")

        : message(message)  // 个性签名
    {
        this->status = status;
        this->id = id;
        this->name = name;
        this->avatar = avatar;
        this->sex = sex;
        this->desc = desc;
    }
    explicit FriendItem(){}
};





#endif // FRIENDITEM_H
