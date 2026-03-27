#ifndef CHATGRPCCLIENT_H
#define CHATGRPCCLIENT_H

#include "../data/UserInfo.h"
#include "../data/im.pb.h"
#include "../global/Singleton.h"
#include "../global/const.h"
#include "RPCPool.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>
#include <nlohmann/json.hpp>
#include <unordered_map>

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

using im::MessageContent;
using im::MessageItem;

using message::KickUserReq;
using message::KickUserRsp;

class ChatGrpcClient : public Singleton<ChatGrpcClient> {
  friend class Singleton<ChatGrpcClient>;

public:
  ~ChatGrpcClient() = default;

  /**
   * @brief 添加好友请求
   *
   * @param server_ip
   * @return AddFriendResponse
   */
  AddFriendResponse NotifyAddFriend(std::string server_ip,
                                    const AddFriendRequest &);
  /**
   * @brief 好友验证请求
   *
   * @param server_ip
   * @return AuthFriendResponse
   */
  AuthFriendResponse NotifyAuthFriend(std::string server_ip,
                                      const AuthFriendRequest &);
  /**
   * @brief 消息发送
   *
   * @param server_ip
   * @param req
   * @return TextChatMessageResponse
   */
  TextChatMessageResponse
  NotifyTextChatMessage(std::string server_ip,
                        const TextChatMessageRequest &req);
  /**
   * @brief 通知好友已经建立关系了
   *
   * @param server_ip
   * @param req
   * @return NotifyMakeFriendsResponse
   */
  NotifyMakeFriendsResponse
  NotifyMakeFriends(std::string server_ip, const NotifyMakeFriendsRequest &req);
  /**
   * @brief 通知好友上线
   *
   * @param server_ip
   * @param req
   * @return NotifyFriendOnlineResponse
   */
  NotifyFriendOnlineResponse
  NotifyFriendOnline(std::string server_ip,
                     const NotifyFriendOnlineRequest &req);
  /**
   * @brief 通知剔除用户
   *
   * @param server_ip
   * @param req
   * @return NotifyFriendOnlineResponse
   */
  KickUserRsp NotifyKickUser(std::string server_ip, const KickUserReq &req);

private:
  ChatGrpcClient();

  std::unordered_map<std::string,
                     std::unique_ptr<RPCPool<ChatServer, ChatServer::Stub>>>
      _pool;
};

#endif
