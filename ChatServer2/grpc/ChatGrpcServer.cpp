#include "../global/UserManager.h"
#include "../server/Server.h"
#include "ChatGrpcServer.h"
#include <spdlog/spdlog.h>
ChatGrpcServer::ChatGrpcServer()
{
}
void ChatGrpcServer::SetServer(std::shared_ptr<Server>server)
{
    this->_server = server;
}

Status ChatGrpcServer::NotifyAddFriend(grpc::ServerContext* context, const AddFriendRequest* request, AddFriendResponse* response)
{
    SPDLOG_INFO("Add Friend Request From {}", request->fromuid());
    // 首先在本服务器查询
    auto to_uid = request->touid();
    auto session = UserManager::GetInstance()->GetSession(to_uid);
    Defer defer([request, response]() {
        response->set_error(static_cast<int>(ErrorCodes::SUCCESS));
        response->set_fromuid(request->fromuid());
        response->set_touid(request->touid());
    });

    // 不在内存中
    if (session == nullptr) {
        return Status::OK;
    }
    // 在内存中z直接发送通知
    json j;
    j["error"] = ErrorCodes::SUCCESS;
    j["from_uid"] = request->fromuid();
    j["name"] = request->name();
    j["icon"] = request->icon();
    j["sex"] = request->sex();
    j["desc"] = request->desc();

    session->Send(j.dump(), static_cast<int>(MsgId::ID_NOTIFY_ADD_FRIEND_REQ));

    return Status::OK;
}
Status ChatGrpcServer::NotifyAuthFriend(grpc::ServerContext* context, const AuthFriendRequest* request, AuthFriendResponse* response)
{
    return Status::OK;
}
Status ChatGrpcServer::NotifyTextChatMessage(grpc::ServerContext* context, const TextChatMessageRequest* request, TextChatMessageResponse* response)
{
    auto to_uid = request->touid();
    auto session = UserManager::GetInstance()->GetSession(to_uid);
    Defer defer([request, response]() {
        response->set_error(static_cast<int>(ErrorCodes::SUCCESS));
    });
    if (session == nullptr) {
        return Status::OK;
    }

    session->Send(request->data(), static_cast<int>(MsgId::ID_NOTIFY_TEXT_CHAT_MSG_REQ));
    return Status::OK;
}

bool ChatGrpcServer::ChatGrpcServer::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
    return true;
}

Status ChatGrpcServer::NotifyMakeFriends(grpc::ServerContext* context, const NotifyMakeFriendsRequest* request, NotifyMakeFriendsResponse* response)
{

    SPDLOG_INFO("Make Friends From {} to {}", request->fromuid(), request->touid());
    // 首先在本服务器查询
    auto to_uid = request->touid();
    auto session = UserManager::GetInstance()->GetSession(to_uid);
    Defer defer([request, response]() {
        response->set_error(static_cast<int>(ErrorCodes::SUCCESS));
        response->set_fromuid(request->fromuid());
        response->set_touid(request->touid());
    });

    // 不在内存中
    if (session == nullptr) {
        return Status::OK;
    }
    // 在内存中z直接发送通知
    json j;
    j["error"] = ErrorCodes::SUCCESS;
    j["to_uid"] = request->touid();
    j["from_uid"] = request->fromuid();
    j["from_name"] = request->fromname();
    j["from_sex"] = request->fromsex();
    j["from_icon"] = request->fromicon();
    j["from_status"] = request->fromstatus();
    j["type"] = request->type();
    j["message"] = request->message();
    j["from_desc"] = request->fromdesc();

    session->Send(j.dump(), static_cast<int>(MsgId::ID_NOTIFY_AUTH_FRIEND_REQ));

    return Status::OK;
}

Status ChatGrpcServer::NotifyFriendOnline(grpc::ServerContext* context, const NotifyFriendOnlineRequest* request, NotifyFriendOnlineResponse* response)
{
    auto to_uid = request->touid();
    auto session = UserManager::GetInstance()->GetSession(to_uid);
    Defer defer([request, response]() {
        response->set_error(static_cast<int>(ErrorCodes::SUCCESS));
    });

    // 不在内存中
    if (session == nullptr) {
        return Status::OK;
    }
    // 在内存中z直接发送通知
    json j;
    j["error"] = ErrorCodes::SUCCESS;
    j["uid"] = request->fromuid();
    j["name"] = request->name();
    j["type"] = request->type();
    j["icon"] = request->icon();
    j["message"] = request->message();
    j["time"] =request->time();

    session->Send(j.dump(), static_cast<int>(MsgId::ID_NOTIFY));

    return Status::OK;
}


Status ChatGrpcServer::NotifyKickUser(grpc::ServerContext*context,const KickUserReq*request,KickUserRsp*response)
{
    auto uid = request->uid();
    auto session = UserManager::GetInstance()->GetSession(uid);
    Defer defer([response]() {
        response->set_error(static_cast<int>(ErrorCodes::SUCCESS));
    });

    // 不在内存中
    if (session == nullptr) {
        return Status::OK;
    }

    session->NotifyOffline(uid);
    _server->ClearSession(session->GetSessionId());

    return Status::OK;
}
