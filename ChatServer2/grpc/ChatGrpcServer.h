#ifndef CHATGRPCSERVER_H
#define CHATGRPCSERVER_H

#include "../data/UserInfo.h"
#include "RPCPool.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <nlohmann/json.hpp>

class Server;


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using message::ChatServer;

using message::AddFriendRequest;
using message::AddFriendResponse;

using message::AuthFriendRequest;
using message::AuthFriendResponse;

using message::GetChatServerRequest;
using message::GetChatServerResponse;

using message::TextChatData;
using message::TextChatMessageRequest;
using message::TextChatMessageResponse;

using message::NotifyMakeFriendsRequest;
using message::NotifyMakeFriendsResponse;

using message::NotifyFriendOnlineRequest;
using message::NotifyFriendOnlineResponse;

using message::KickUserReq;
using message::KickUserRsp;

class ChatGrpcServer final : public message::ChatServer::Service {
public:
    ChatGrpcServer();

    /**
     * @brief 设置服务器
     * @param server
     */
    void SetServer(std::shared_ptr<Server>server);
    /**
    * @brief 用户基本信息回包
    *
    * @param base_key
    * @param uid
    * @param userinfo
    * @return true
    * @return false
    */
    bool GetBaseInfo(std::string base_key, int uid,
                    std::shared_ptr<UserInfo> &userinfo);
    /**
    * @brief 好友请求回包
    *
    * @param context
    * @param request
    * @param response
    * @return Status
    */
    Status NotifyAddFriend(grpc::ServerContext *context,
                            const AddFriendRequest *request,
                            AddFriendResponse *response) override;
    /**
    * @brief 好友验证回包
    *
    * @param context
    * @param request
    * @param response
    * @return Status
    */
    Status NotifyAuthFriend(grpc::ServerContext *context,
                            const AuthFriendRequest *request,
                            AuthFriendResponse *response) override;
    /**
    * @brief 聊天消息回包
    *
    * @param context
    * @param request
    * @param response
    * @return Status
    */
    Status NotifyTextChatMessage(grpc::ServerContext *context,
                                const TextChatMessageRequest *request,
                                TextChatMessageResponse *response) override;
    /**
    * @brief 通知好友已经建立好友关系
    *
    * @param server_ip
    * @param req
    * @return NotifyMakeFriendsResponse
    */
    Status NotifyMakeFriends(grpc::ServerContext *context,
                            const NotifyMakeFriendsRequest *request,
                            NotifyMakeFriendsResponse *response) override;
    /**
    * @brief 通知好友上线
    *
    * @param context
    * @param request
    * @param response
    * @return Status
    */
    Status NotifyFriendOnline(grpc::ServerContext *context,
                                const NotifyFriendOnlineRequest *request,
                                NotifyFriendOnlineResponse *response) override;
    /**
    * @brief 通知踢人
    *
    * @param context
    * @param request
    * @param response
    * @return Status
    */
    Status NotifyKickUser(grpc::ServerContext*context,const KickUserReq*request,KickUserRsp*response) override;

private:
    std::shared_ptr<Server>_server;
};

#endif
