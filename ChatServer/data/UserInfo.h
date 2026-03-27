#pragma once
#include <string>

struct UserInfo {
    UserInfo()
        : name("")
        , password("")
        , email("")
        , nick("")
        , desc("")
        , icon("")
        , back("") // 备用字段
        , token("")
        , status(0)
        , sex(0)
        , uid(0)
    {
    }

    std::string name;
    std::string password;
    std::string email;
    std::string nick;
    std::string desc;
    std::string icon;
    std::string back;
    std::string token;
    int status;
    int uid;
    int sex;
};

struct ApplyInfo {
    ApplyInfo() { }
    ApplyInfo(int uid, const std::string& name, const std::string& desc, const std::string& nick, const std::string& icon, const std::string& email, int sex)
        : uid(uid)
        , name(name)
        , desc(desc)
        , nick(nick)
        , icon(icon)
        , email(email)
        , sex(sex)
    {
    }

    std::string name;
    std::string desc;
    std::string nick;
    std::string icon;
    std::string email;
    int uid;
    int sex;
};

struct SessionInfo {
    std::string uid;
    int from_uid;
    int to_uid;
    std::string create_time;
    std::string update_time;
    std::string name;
    std::string icon;
    int status;
    int deleted;
    int pined;
    bool processed;
    SessionInfo() { }
};