#include "ChatGrpcClient.h"
#include "../global/ConfigManager.h"
#include "message.pb.h"
#include <grpcpp/client_context.h>
#include <spdlog/spdlog.h>
#include <string>

AddFriendResponse ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendRequest& req)
{
    SPDLOG_INFO("发送好友请求to:{}", req.touid());
    SPDLOG_INFO("目标服务名称:{}", server_ip);

    AddFriendResponse rsp;
    Defer defer([&rsp, &req]() {
        rsp.set_error(static_cast<int>(ErrorCodes::SUCCESS));
        rsp.set_fromuid(req.fromuid());
        rsp.set_touid(req.touid());
    });

    auto it = _pool.find(server_ip);
    if (it == _pool.end()) {
        return rsp;
    }

    auto& pool = it->second;
    SPDLOG_INFO("服务端ip,{}:{}", _pool[server_ip]->_host, _pool[server_ip]->_port);
    grpc::ClientContext context;
    auto stub = pool->GetConnection();
    Defer defer2([&pool, &stub]() {
        pool->ReturnConnection(std::move(stub));
    });

    Status status = stub->NotifyAddFriend(&context, req, &rsp);
    if (!status.ok()) {
        rsp.set_error(static_cast<int>(ErrorCodes::RPCFAILED));
        return rsp;
    }

    return rsp;
}

AuthFriendResponse ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendRequest&)
{
    AuthFriendResponse rsp;

    return rsp;
}

TextChatMessageResponse ChatGrpcClient::NotifyTextChatMessage(std::string server_ip, const TextChatMessageRequest& req)
{
    TextChatMessageResponse rsp;
    Defer defer([&rsp, &req]() {
        rsp.set_error(static_cast<int>(ErrorCodes::SUCCESS));
    });

    auto it = _pool.find(server_ip);
    if (it == _pool.end()) {
        return rsp;
    }

    auto& pool = it->second;
    grpc::ClientContext context;
    auto stub = pool->GetConnection();
    Defer defer2([&pool, &stub]() {
        pool->ReturnConnection(std::move(stub));
    });

    Status status = stub->NotifyTextChatMessage(&context, req, &rsp);
    if (!status.ok()) {
        rsp.set_error(static_cast<int>(ErrorCodes::RPCFAILED));
        return rsp;
    }

    return rsp;
}

ChatGrpcClient::ChatGrpcClient()
{
    auto& cfg = ConfigManager::GetInstance();
    auto server_list = cfg["PeerServer"]["servers"];

    std::vector<std::string> words;
    words.reserve(10);

    std::stringstream ss(server_list);
    std::string word;

    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }

    for (const auto& word : words) {
        if (cfg[word]["name"].empty()) {
            continue;
        }
        _pool[cfg[word]["name"]] = std::make_unique<RPCPool<ChatServer, ChatServer::Stub>>(10, cfg[word]["host"], cfg[word]["RPCPort"]);
    }
}

NotifyMakeFriendsResponse ChatGrpcClient::NotifyMakeFriends(std::string server_ip, const NotifyMakeFriendsRequest& req)
{

    SPDLOG_INFO("发送好友请求to:{}", req.touid());
    SPDLOG_INFO("目标服务名称:{}", server_ip);

    NotifyMakeFriendsResponse rsp;
    Defer defer([&rsp, &req]() {
        rsp.set_error(static_cast<int>(ErrorCodes::SUCCESS));
        rsp.set_fromuid(req.fromuid());
        rsp.set_touid(req.touid());
    });

    auto it = _pool.find(server_ip);
    if (it == _pool.end()) {
        return rsp;
    }

    auto& pool = it->second;
    // SPDLOG_INFO("服务端ip,{}:{}", _pool[server_ip]->_host, _pool[server_ip]->_port);
    grpc::ClientContext context;
    auto stub = pool->GetConnection();
    Defer defer2([&pool, &stub]() {
        pool->ReturnConnection(std::move(stub));
    });

    Status status = stub->NotifyMakeFriends(&context, req, &rsp);
    if (!status.ok()) {
        rsp.set_error(static_cast<int>(ErrorCodes::RPCFAILED));
        return rsp;
    }

    return rsp;
}

NotifyFriendOnlineResponse ChatGrpcClient::NotifyFriendOnline(std::string server_ip, const NotifyFriendOnlineRequest& req)
{
    NotifyFriendOnlineResponse rsp;
    Defer defer([&rsp, &req]() {
        rsp.set_error(static_cast<int>(ErrorCodes::SUCCESS));
    });

    auto it = _pool.find(server_ip);
    if (it == _pool.end()) {
        return rsp;
    }

    auto& pool = it->second;
    // SPDLOG_INFO("服务端ip,{}:{}", _pool[server_ip]->_host, _pool[server_ip]->_port);
    grpc::ClientContext context;
    auto stub = pool->GetConnection();
    Defer defer2([&pool, &stub]() {
        pool->ReturnConnection(std::move(stub));
    });

    Status status = stub->NotifyFriendOnline(&context, req, &rsp);
    if (!status.ok()) {
        rsp.set_error(static_cast<int>(ErrorCodes::RPCFAILED));
        return rsp;
    }
    return rsp;
}

KickUserRsp ChatGrpcClient::NotifyKickUser(std::string server_ip, const KickUserReq &req)
{
    KickUserRsp rsp;
    rsp.set_error(static_cast<int>(ErrorCodes::RPCFAILED));
    Defer defer([&rsp, &req]() {
        rsp.set_uid(req.uid());
    });

    auto it = _pool.find(server_ip);
    if (it == _pool.end()) {
        return rsp;
    }

    auto& pool = it->second;
    grpc::ClientContext context;
    auto stub = pool->GetConnection();
    Defer defer2([&pool, &stub]() {
        pool->ReturnConnection(std::move(stub));
    });

    Status status = stub->NotifyKickUser(&context, req, &rsp);
    if (!status.ok()) {
        return rsp;
    }
    rsp.set_error(static_cast<int>(ErrorCodes::SUCCESS));
    return rsp;
}
