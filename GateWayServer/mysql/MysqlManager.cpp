#include "MysqlManager.h"
MysqlManager::~MysqlManager()
{
}

int MysqlManager::TestUidAndEmail(const std::string& uid, const std::string& email)
{
    return _dao.TestUidAndEmail(uid, email);
}

MysqlManager::MysqlManager()
{
}

int MysqlManager::RegisterUser(const std::string& name, const std::string& email, const std::string& password)
{
    return _dao.RegisterUser(name, email, password);
}

int MysqlManager::ResetPassword(const std::string& email, const std::string& password)
{
    return _dao.ResetPassword(email, password);
}

bool MysqlManager::CheckPwd(const std::string& user, const std::string& password, UserInfo& userInfo)
{
    return _dao.CheckPwd(user, password, userInfo);
}

// bool MysqlManager::CheckEmail(const std::string& name, const std::string& email)
// {
// }

// bool MysqlManager::UpdatePassword(const std::string& name, const std::string& email)
// {
// }

// bool MysqlManager::CheckPassword(const std::string& email, const std::string& pwd, UserInfo& userInfo)
// {
// }

// bool MysqlManager::TestProcedure(const std::string& name, const std::string& email)
// {
// }
