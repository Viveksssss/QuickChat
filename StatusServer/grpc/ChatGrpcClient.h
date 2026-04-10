#ifndef CHATGRPCCLIENT_H
#define CHATGRPCCLIENT_H

#include "../global/ConfigManager.h"
#include "../global/Singleton.h"
#include "../global/const.h"
#include "RPCPool.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <nlohmann/json.hpp>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>
#include <unordered_map>


using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::ChatServer;

using message::AddFriendRequest;
using message::AddFriendResponse;

using message::AuthFriendRequest;
using message::AuthFriendResponse;

using message::GetChatServerResponse;
using message::GetChatServerRequest;

using message::TextChatMessageRequest;
using message::TextChatMessageResponse;
using message::TextChatData;

class ChatGrpcClient : public Singleton<ChatGrpcClient> {
    friend class Singleton<ChatGrpcClient>;

public:
    ~ChatGrpcClient() = default;
    GetChatServerResponse NotifyAddFriend(std::string server_ip,const AddFriendRequest&);

private:
    ChatGrpcClient();

    std::unordered_map<std::string,std::unique_ptr<RPCPool<ChatServer, ChatServer::Stub>>>_pool;
};



#endif
