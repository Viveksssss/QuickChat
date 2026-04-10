#ifndef STATUSSERVICEIMPL_H
#define STATUSSERVICEIMPL_H

#include "message.grpc.pb.h"
#include <grpc++/grpc++.h>
#include <queue>
#include <unordered_map>

using grpc::Server;

struct ChatServer {
    std::string host;
    std::string port;
    std::string name;
    int con_count;
};

class StatusServiceImpl final : public message::StatusService::Service {
public:
    StatusServiceImpl();
    grpc::Status GetChatServer(grpc::ServerContext* context, const message::GetChatServerRequest* request, message::GetChatServerResponse* response) override;
    grpc::Status Login(grpc::ServerContext* context, const message::LoginRequest* request, message::LoginResponse* response) override;

private:
    void insertToken(int uid, const std::string& token);
    ChatServer GetChatServer();

private:
    struct CompareServers {
        bool operator()(const ChatServer& a, const ChatServer& b) const
        {
            return a.con_count < b.con_count;
        }
    };

    // std::priority_queue<ChatServer, std::vector<ChatServer>, CompareServers> _servers;
    std::unordered_map<std::string, ChatServer>_servers;
    std::mutex _server_mutex;

};

#endif
