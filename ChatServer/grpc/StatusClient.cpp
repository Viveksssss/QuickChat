#include "StatusClient.h"

StatusClient::StatusClient()
{
    auto& cfgMgr = ConfigManager::GetInstance();
    std::string host = cfgMgr["StatusServer"]["host"];
    std::string port = cfgMgr["StatusServer"]["port"];
    _pool = std::make_unique<RPCPool<StatusService, StatusService::Stub>>(10, host, port);
}

GetChatServerResponse StatusClient::GetChatServer(int uid)
{
    grpc::ClientContext context;
    GetChatServerResponse reply;
    GetChatServerRequest request;
    request.set_uid(uid);
    auto stub = _pool->GetConnection();
    grpc::Status status = stub->GetChatServer(&context, request, &reply);
    Defer defer([this, &stub]() mutable {
        _pool->ReturnConnection(std::move(stub));
    });
    if (!status.ok()) {
        reply.set_error(status.error_code());
    }
    return reply;
}

LoginResponse StatusClient::Login(int uid, const std::string& token)
{
    grpc::ClientContext context;
    LoginRequest request;
    LoginResponse reply;
    request.set_uid(uid);
    request.set_token(token);
    auto stub = _pool->GetConnection();
    grpc::Status status = stub->Login(&context, request, &reply);
    Defer defer([this, &stub]() mutable {
        _pool->ReturnConnection(std::move(stub));
    });
    if (!status.ok()) {
        reply.set_error(status.error_code());
    }
    return reply;
}
