#ifndef MYSQLMANAGER_H
#define MYSQLMANAGER_H

#include "../global/Singleton.h"
#include "MysqlDao.h"
#include <mysql/mysql.h>

struct UserInfo;
class MysqlManager : public Singleton<MysqlManager> {
    friend class Singleton<MysqlManager>;

public:
    ~MysqlManager();

    int TestUidAndEmail(const std::string& uid, const std::string& email);
    int RegisterUser(const std::string& name, const std::string& email, const std::string& password);
    int ResetPassword(const std::string& email, const std::string& password);
    bool CheckPwd(const std::string& user, const std::string& password, UserInfo& userInfo);

private:
    MysqlManager();

private:
    MysqlDao _dao;
};

#endif // MYSQLMANAGER_H