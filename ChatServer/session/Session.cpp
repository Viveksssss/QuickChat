#include "../session/Session.h"
#include "../global/const.h"
#include "../redis/RedisManager.h"
#include "../server/LogicSystem.h"
#include "../server/Server.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Session.h"
#include <chrono>
#include <ctime>
#include <spdlog/spdlog.h>

Session::Session(boost::asio::io_context &ioc, std::shared_ptr<Server> server)
    : _socket(ioc)
    , _server(server)
    , _stop(false)
    , _head_parse(false)
    , _uid(0)
    , _last_heartbeat(std::time(nullptr)) {
    _session_id = boost::uuids::to_string(boost::uuids::random_generator()());
    _recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

Session::~Session() { Close(); }

void Session::Start() {
    _last_heartbeat = std::time(nullptr);
    AsyncHead(HEAD_TOTAL_LEN);
}

void Session::Close() {

    if (_stop) {
        return;
    }
    _stop = true;
    {
        std::lock_guard<std::mutex> lock(_send_lock);
        while (!_send_queue.empty()) {
            _send_queue.pop();
        }
    }

    // 安全关闭socket
    boost::system::error_code ec;
    ec = _socket.close(ec);
    if (ec) {
        SPDLOG_WARN("Socket shutdown error: {}", ec.message());
    }

    ec = _socket.close(ec);
    if (ec) {
        SPDLOG_WARN("Socket close error: {}", ec.message());
    }
    SPDLOG_INFO("Session {} disconnected!", _session_id);
}

void Session::DealExceptionSession() {
    auto uid_str = std::to_string(_uid);
    auto lock_key = LOCK_PREFIX + uid_str;
    auto identifier = RedisManager::GetInstance()->AcquireLock(
        lock_key, LOCK_TIMEOUT, ACQUIRE_LOCK_TIMEOUT);

    Defer defer([identifier, lock_key, this]() {
        _server->ClearSession(_session_id);
        RedisManager::GetInstance()->ReleaseLock(lock_key, identifier);
    });

    if (identifier.empty()) {
        return;
    }

    std::string redis_session_id = "";
    bool b_session = RedisManager::GetInstance()->Get(
        USER_SESSION_PREFIX + uid_str, redis_session_id);
    if (!b_session) {
        // 没有查询到，说明没有登陆信息
        return;
    }
    if (redis_session_id != _session_id) {
        // 说明有客户在其他的服务器异地登录了
        // 这时候不需要修改/删除信息，这时候已经是新登陆的信息了
        return;
    }

    // 删除session
    RedisManager::GetInstance()->Del(USER_SESSION_PREFIX + uid_str);
    // 删除用户服务器信息
    RedisManager::GetInstance()->Del(USERIP_PREFIX + std::to_string(_uid));
    // 修改用户的状态
    RedisManager::GetInstance()->Set(USER_STATUS_PREFIX + std::to_string(_uid),
                                     std::to_string(0));
}

void Session::NotifyOffline(int uid) {
    json j;
    j["error"] = ErrorCodes ::SUCCESS;
    j["uid"] = uid;
    Send(j.dump(), static_cast<uint16_t>(MsgId::ID_NOTIFY_OFF_LINE_REQ));
    return;
}

void Session::Send(const char *msg, int max_length, uint16_t msg_id) {
    std::lock_guard<std::mutex> lock(_send_lock);
    int size = _send_queue.size();
    if (size > MAX_SEND_SIZE) {
        SPDLOG_WARN("Session:{} Send Queue Fulled,This Failed", _session_id);
        return;
    }
    _send_queue.push(std::make_shared<SendNode>(msg, max_length, msg_id));
    if (size > 0) {
        return;
    }
    auto msgnode = _send_queue.front();
    boost::asio::async_write(
        _socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
        std::bind(&Session::HandleWrite, this, std::placeholders::_1,
                  shared_from_this()));
}

void Session::Send(std::string msg, uint16_t msg_id) {
    Send(msg.data(), msg.size(), msg_id);
}

void Session::HandleWrite(boost::system::error_code ec,
                          std::shared_ptr<Session> self) {
    try {
        if (ec) {
            SPDLOG_WARN("Handle Write Filed,Errir is {}", ec.what());
            Close();
            DealExceptionSession();
        }
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_queue.pop();
        if (!_send_queue.empty()) {
            auto msgnode = _send_queue.front();
            boost::asio::async_write(
                _socket,
                boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                std::bind(&Session::HandleWrite, this, std::placeholders::_1,
                          shared_from_this()));
        }

    } catch (std::exception &e) {
        SPDLOG_WARN("Exception code:{}", e.what());
        Close();
        DealExceptionSession();
    }
}

net::ip::tcp::socket &Session::GetSocket() noexcept { return _socket; }

std::queue<std::shared_ptr<SendNode>> &Session::GetSendQueue() noexcept {
    return _send_queue;
}

void Session::SetUid(int uid) noexcept { this->_uid = uid; }

int Session::GetUid() const noexcept { return this->_uid; }

std::string Session::GetSessionId() const noexcept { return _session_id; }

void Session::SetSessionId(const std::string &session_id) noexcept {
    this->_session_id = session_id;
}

void Session::AsyncHead(std::size_t len) {
    boost::asio::async_read(
        _socket, boost::asio::buffer(_recv_head_node->_data, len),
        [self = shared_from_this(), this](boost::system::error_code error,
                                          size_t bytes_transferred) {
            if (error) {
                if (error == boost::asio::error::eof) {
                    SPDLOG_WARN("Connection closed by perr");
                } else {
                    SPDLOG_WARN("Error Reading header:{},bytes transferred:{}",
                                error.message(), bytes_transferred);
                }
                Close();
                DealExceptionSession();
                return;
            }
            if (bytes_transferred < HEAD_TOTAL_LEN) {
                SPDLOG_WARN("Read Length not matched");
                Close();
                DealExceptionSession();
                return;
            }
            if (!_server->CheckValid(_session_id)) {
                SPDLOG_WARN("Invalid SessionId");
                Close();
                DealExceptionSession();
                return;
            }

            // 更新心跳
            UpdateHeartbeat();

            uint16_t msg_id = 0;
            memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
            msg_id =
                boost::asio::detail::socket_ops::network_to_host_short(msg_id);
            uint16_t data_len = 0;
            memcpy(&data_len, _recv_head_node->_data + HEAD_ID_LEN,
                   HEAD_DATA_LEN);
            data_len = boost::asio::detail::socket_ops::network_to_host_short(
                data_len);

            _recv_msg_node = std::make_shared<RecvNode>(data_len, msg_id);
            AsyncBody(data_len);
        });
}

void Session::AsyncBody(std::size_t len) {
    boost::asio::async_read(
        _socket, boost::asio::buffer(_data, len),
        [self = shared_from_this(), this](boost::system::error_code error,
                                          size_t bytes_transferred) {
            if (error) {
                if (error == boost::asio::error::eof) {
                    SPDLOG_WARN("Connection closed by perr");
                } else {
                    SPDLOG_WARN("Error Reading Body:{},bytes transferred:{}",
                                error.message(), bytes_transferred);
                }
                Close();
                DealExceptionSession();
                return;
            }
            if (bytes_transferred < HEAD_TOTAL_LEN) {
                SPDLOG_WARN("Read Length not matched");
                Close();
                DealExceptionSession();
                return;
            }
            if (!_server->CheckValid(_session_id)) {
                SPDLOG_WARN("Invalid SessionId");
                Close();
                return;
            }
            // 更新心跳
            UpdateHeartbeat();

            memcpy(_recv_msg_node->_data, _data, bytes_transferred);
            _recv_msg_node->_cur_len += bytes_transferred;

            auto logic_node =
                std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node);
            LogicSystem::GetInstance()->PostMsgToQueue(logic_node);

            /* 继续接受head->body->head->body... */
            AsyncHead(HEAD_TOTAL_LEN);
        });
}

void Session::UpdateHeartbeat() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    this->_last_heartbeat = now_t;
}
bool Session::IsHeartbeatExpired(std::time_t &now) {
    double diff_sec = std::difftime(now, _last_heartbeat);
    if (diff_sec > 20) {
        SPDLOG_WARN("Heartbeat expired of {}", this->_session_id);
        return true;
    }
    return false;
}

LogicNode::LogicNode(std::shared_ptr<Session> session,
                     std::shared_ptr<RecvNode> recv_node)
    : _session(session)
    , _recv_node(recv_node) {}
