#ifndef STATUCCLIENT_H
#define STATUCCLIENT_H

#include "../global/ConfigManager.h"
#include "../global/Singleton.h"
#include "../global/const.h"
#include "RPCPool.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

using message::GetChatServerRequest;
using message::GetChatServerResponse;
using message::LoginRequest;
using message::LoginResponse;
using message::StatusService;

class StatusClient : public Singleton<StatusClient> {
    friend class Singleton<StatusClient>;

public:
    ~StatusClient() = default;
    GetChatServerResponse GetChatServer(int uid);
    LoginResponse Login(int uid, const std::string& token);

private:
    StatusClient();

    std::unique_ptr<RPCPool<StatusService, StatusService::Stub>> _pool;
};

#endif