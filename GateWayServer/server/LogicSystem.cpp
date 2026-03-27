#include "../server/LogicSystem.h"
#include "../grpc/StatusClient.h"
#include "../grpc/VerifyClient.h"
#include "../redis/RedisManager.h"
#include "../session/Session.h"
#include <regex>
#include <spdlog/spdlog.h>

#include "../mysql/MysqlManager.h"

bool LogicSystem::HandleGet(const std::string& route, std::shared_ptr<Session> handler_ptr)
{
    if (_get_handlers.find(route) == _get_handlers.end()) {
        return false;
    }
    _get_handlers[route](handler_ptr);
    return true;
}

bool LogicSystem::HandlePost(const std::string& route, std::shared_ptr<Session> handler_ptr)
{
    if (_post_handlers.find(route) == _post_handlers.end()) {
        return false;
    }
    _post_handlers[route](handler_ptr);
    return true;
}

void LogicSystem::RegistHandlers(const std::string& route, RequestType type, SessionHandler handler)
{
    type == RequestType::GET ? _get_handlers[route] = handler : _post_handlers[route] = handler;
}

LogicSystem::LogicSystem()
{
    initHandlers();
}

void LogicSystem::initHandlers()
{
    RegistHandlers("/get_test", RequestType::GET, [this](std::shared_ptr<Session> session) {
            beast::ostream(session->_response.body()) << "receive login request\n";
            int i = 0;
            for(const auto&ele:session->_get_params ){
                i++;
                beast::ostream(session->_response.body()) << i <<"\t" << ele.first << ":" << ele.second << "\n";
            } });
    RegistHandlers("/getSecurityCode", RequestType::POST, [this](std::shared_ptr<Session> session) {
        auto body_str = beast::buffers_to_string(session->_request.body().data());
        SPDLOG_DEBUG("receive getSecurityCode request, body: {}", body_str);
        session->_response.set(http::field::content_type, "text/json");

        // json解析
        json j = json::parse(body_str);
        if (j.is_discarded()) {
            SPDLOG_WARN("Invalid json");
            j["error"] = ErrorCodes::ERROR_JSON;
            std::string returnJson = j.dump(4);
            beast::ostream(session->_response.body()) << returnJson;
            return true;
        }

        auto sendError = [&](ErrorCodes error_code, const std::string& message) {
            json error_response = {
                { "success", false },
                { "error", error_code },
                { "message", message }
            };
            beast::ostream(session->_response.body()) << error_response.dump(4);
            return true; // 返回true表示请求处理完成
        };

        if (!j.contains("email")) {
            return sendError(ErrorCodes::RPCFAILED, "email is required");
        }

        auto email = j["email"].get<std::string>();

        // 验证格式
        std::regex email_regex(R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b)");
        if (!std::regex_match(email, email_regex)) {
            return sendError(ErrorCodes::RPCFAILED, "email format error");
        }

        GetSecurityCodeResponse response = VerifyClient::GetInstance()->GetSecurityCode(email);


        // 发送验证码
        json returnJson = {
            { "success", true },
            { "error", ErrorCodes::SUCCESS },
            { "message", "okle!" }
        };
        beast::ostream(session->_response.body()) << returnJson.dump(4);
        return true;
    });

    RegistHandlers("/userRegister", RequestType::POST, [this](std::shared_ptr<Session> session) {
        auto body_str = beast::buffers_to_string(session->_request.body().data());
        SPDLOG_DEBUG("receive userRegister request, body: {}", body_str);
        session->_response.set(http::field::content_type, "text/json");
        json j = json::parse(body_str);
        if (j.is_null() || j.is_discarded()) {
            SPDLOG_WARN("Invalid json");
            j["error"] = ErrorCodes::ERROR_JSON;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }

        // 先查找用户是否存在
        // 这里简单测试注册功能
        auto email = j["email"].get<std::string>();
        auto name = j["user"].get<std::string>();
        auto password = j["password"].get<std::string>();
        auto securityCode = j["securityCode"].get<std::string>();

        std::string code;
        bool get_code_success = RedisManager::GetInstance()->Get(EMAIL_PREFIX + email, code);
        if (!get_code_success) {
            SPDLOG_WARN("Security code expired");
            j["error"] = ErrorCodes::ERROR_SECURITYCODE_EXPIRED;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        } else if (code != securityCode) {

            SPDLOG_WARN("Security code not found");
            j["error"] = ErrorCodes::ERROR_SECURITYCODE_NOTFOUND;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }
        int uid = MysqlManager::GetInstance()->RegisterUser(name, email, password);
        if (uid == 0 || uid == -1) {
            SPDLOG_WARN("Failed to register user: {}", email);
            j["error"] = ErrorCodes::RPCFAILED;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }

        j["success"] = true;
        j["error"] = ErrorCodes::SUCCESS;
        j["message"] = "注册成功";
        std::string str = j.dump(4);
        beast::ostream(session->_response.body()) << str;
        return true;
    });
    RegistHandlers("/resetPassword", RequestType::POST, [this](std::shared_ptr<Session> session) {
        auto body_str = beast::buffers_to_string(session->_request.body().data());
        SPDLOG_DEBUG("receive resetPassword request, body: {}", body_str);
        session->_response.set(http::field::content_type, "text/json");
        json j = json::parse(body_str);
        if (j.is_null() || j.is_discarded()) {
            SPDLOG_WARN("Invalid json");
            j["error"] = ErrorCodes::ERROR_JSON;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }

        auto uid = j["uid"].get<std::string>();
        auto email = j["email"].get<std::string>();
        auto password = j["password"].get<std::string>();
        auto securityCode = j["securityCode"].get<std::string>();

        bool ok = MysqlManager::GetInstance()->TestUidAndEmail(uid, email);
        if (!ok) {
            SPDLOG_WARN("Failed to reset password for user: {}", email);
            j["error"] = ErrorCodes::ERROR_EMAIL_NOTFOUND;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }

        std::string code;
        bool get_code_success = RedisManager::GetInstance()->Get(EMAIL_PREFIX + email, code);
        if (!get_code_success) {
            SPDLOG_WARN("Security code expired");
            j["error"] = ErrorCodes::ERROR_SECURITYCODE_EXPIRED;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        } else if (code != securityCode) {
            SPDLOG_WARN("Security code not found");
            j["error"] = ErrorCodes::ERROR_SECURITYCODE_NOTFOUND;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }
        int res = MysqlManager::GetInstance()->ResetPassword(email, password);
        if (res == 0 || res == -1) {
            SPDLOG_WARN("Failed to reset password for user: {}", email);
            j["error"] = ErrorCodes::RPCFAILED;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }

        j["success"] = true;
        j["error"] = ErrorCodes::SUCCESS;
        j["message"] = "密码修改成功";
        std::string str = j.dump(4);
        beast::ostream(session->_response.body()) << str;
        return true;
    });

    RegistHandlers("/userLogin", RequestType::POST, [this](std::shared_ptr<Session> session) {
        auto body_str = beast::buffers_to_string(session->_request.body().data());
        SPDLOG_DEBUG("receive userLogin request, body: {}", body_str);
        session->_response.set(http::field::content_type, "text/json");
        json j = json::parse(body_str);

        if (j.is_null() || j.is_discarded()) {
            SPDLOG_WARN("Invalid json");
            j["error"] = ErrorCodes::ERROR_JSON;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }

        auto user = j["user"].get<std::string>();
        auto password = j["password"].get<std::string>();

        // 验证密码得到uid
        UserInfo userInfo;
        bool ok = MysqlManager::GetInstance()->CheckPwd(user, password, userInfo);
        if (!ok) {
            SPDLOG_WARN("Failed to login user: {}", user);
            j["error"] = ErrorCodes::ERROR_USER_OR_PASSWORD_INCORRECT;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }
        // 登录成功，根据uid寻找服务器
        auto reply = StatusClient::GetInstance()->GetChatServer(userInfo.uid);
        if (reply.error()) {
            SPDLOG_ERROR("Failed to load user chat server: {}", reply.error());
            j["error"] = ErrorCodes::RPCFAILED;
            std::string str = j.dump(4);
            beast::ostream(session->_response.body()) << str;
            return true;
        }

        SPDLOG_INFO("User {} login success.", userInfo.uid);
        j["token"] = reply.token();
        j["host"] = reply.host();
        j["port"] = reply.port();
        j["uid"] = userInfo.uid;
        j["name"] = userInfo.name;
        j["email"] = userInfo.email;

        j["error"] = reply.error();
        j["message"] = "登录成功";

        beast::ostream(session->_response.body()) << j.dump(4);
        return true;
    });
}
