#ifndef SESSION_H
#define SESSION_H

#include "../global/const.h"
#include "MsgNode.h"
#include <memory>
#include <queue>

class Server;
class LogicSystem;

namespace net = boost::asio;

class Session : public std::enable_shared_from_this<Session> {
    friend class LogicSystem;

  public:
    Session(boost::asio::io_context &ioc, std::shared_ptr<Server> server);
    ~Session();
    void Start();
    void Close();
    void DealExceptionSession();
    void Send(const char *msg, int max_length, uint16_t msg_id);
    void Send(std::string msg, uint16_t msg_id);
    void NotifyOffline(int uid);
    void HandleWrite(boost::system::error_code ec,
                     std::shared_ptr<Session> self);
    net::ip::tcp::socket &GetSocket() noexcept;
    std::queue<std::shared_ptr<SendNode>> &GetSendQueue() noexcept;

    void SetUid(int uid) noexcept;
    int GetUid() const noexcept;
    std::string GetSessionId() const noexcept;
    void SetSessionId(const std::string &session_id) noexcept;

    void UpdateHeartbeat();                    // 更新心跳时间
    bool IsHeartbeatExpired(std::time_t &now); // 是否过期
    std::time_t _last_heartbeat = 0;

  private:
    void AsyncHead(std::size_t len);
    void AsyncBody(std::size_t len);

  private:
    std::shared_ptr<RecvNode> _recv_msg_node;
    std::shared_ptr<MsgNode> _recv_head_node;
    char _data[MAX_LENGTH];

    std::queue<std::shared_ptr<SendNode>> _send_queue;
    std::mutex _send_lock;

    net::ip::tcp::socket _socket;
    std::string _session_id;
    int _uid;

    std::shared_ptr<Server> _server;
    bool _stop;
    bool _head_parse;
};

class LogicNode {
    friend class LogicSystem;

  public:
    LogicNode(std::shared_ptr<Session>, std::shared_ptr<RecvNode>);

  private:
    std::shared_ptr<Session> _session;
    std::shared_ptr<RecvNode> _recv_node;
};

#endif
