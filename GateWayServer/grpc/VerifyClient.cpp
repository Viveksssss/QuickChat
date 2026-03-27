#include "VerifyClient.h"

GetSecurityCodeResponse VerifyClient::GetSecurityCode(const std::string& email)
{
    grpc::ClientContext context;
    GetSecurityCodeRequest request;
    GetSecurityCodeResponse response;
    request.set_email(email);

    auto stub = _pool->GetConnection();
    grpc::Status status = stub->GetSecurityCode(&context, request, &response);
    Defer defer([this, &stub]() {
        _pool->ReturnConnection(std::move(stub));
    });

    if (!status.ok()) {
        response.set_error(static_cast<int>(ErrorCodes::RPCFAILED));
    }

    return response;
}

VerifyClient::VerifyClient()
{
    auto& cfgMgr = ConfigManager::GetInstance();
    std::string host = cfgMgr["VarifyServer"]["host"];
    std::string port = cfgMgr["VarifyServer"]["port"];
    _pool = std::make_unique<RPCPool<VarifyService, VarifyService::Stub>>(10, host, port);
}