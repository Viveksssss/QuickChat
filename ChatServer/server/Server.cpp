#include "Server.h"
#include "../global/ConfigManager.h"
#include "../global/UserManager.h"
#include "../global/const.h"
#include "../redis/RedisManager.h"
#include "../session/Session.h"
#include "AsioPool.h"
#include "LogicSystem.h"
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <mutex>
#include <spdlog/spdlog.h>
#include <unordered_map>

Server::Server(net::io_context &ioc, uint16_t port)
    : _ioc(ioc)
    , _acceptor(ioc, net::ip::tcp::endpoint(net::ip::tcp::v4(), port))
    , _port(port)
    , _timer(_ioc, std::chrono::seconds(30)) {
    SPDLOG_INFO("Server Start Success,Listen on port:{}", _port);
    auto &cfg = ConfigManager::GetInstance();
    _server_name = cfg["SelfServer"]["name"];
}

Server::~Server() {
    StopTimer();
    _sessions.clear();
}

void Server::Start() {
    auto &io_context = AsioPool::GetInstance()->GetIOService();
    std::shared_ptr<Session> conn =
        std::make_shared<Session>(io_context, shared_from_this());
    _acceptor.async_accept(
        conn->GetSocket(), [this, conn, self = shared_from_this()](
                               const boost::system::error_code &ec) {
            try {
                if (ec) {
                    self->Start();
                    return;
                }
                if (!_timer_running) {
                    self->StartTimer();
                }

                conn->Start();
                SPDLOG_INFO(
                    "New connection from {},session:{}",
                    conn->GetSocket().remote_endpoint().address().to_string(),
                    conn->GetSessionId());

                std::unique_lock<std::mutex> lock(_mutex);
                _sessions.insert(std::make_pair(conn->GetSessionId(), conn));
                lock.unlock();

                self->Start();

            } catch (std::exception &e) {
                SPDLOG_ERROR("Exception: {}", e.what());
            }
        });
}

void Server::ClearSession(const std::string &session_id) {
    if (CheckValid(session_id)) {
        UserManager::GetInstance()->RemoveUserSession(
            _sessions[session_id]->GetUid());
    }

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _sessions.erase(session_id);
    }
}

bool Server::CheckValid(const std::string &session_id) {
    auto it = _sessions.find(session_id);
    if (it == _sessions.end()) {
        return false;
    }
    return true;
}

void Server::on_timer(const boost::system::error_code &ec) {
    if (ec) {
        SPDLOG_WARN("Timer error:{}", ec.message());
        return;
    }

    std::vector<std::shared_ptr<Session>> expired_sessions;
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_copy;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        sessions_copy = _sessions;
    }
    int count = 0;
    auto now = std::time(nullptr);
    for (auto it = sessions_copy.begin(); it != sessions_copy.end(); ++it) {
        auto b_expired = it->second->IsHeartbeatExpired(now);
        if (b_expired) {
            it->second->Close();
            expired_sessions.push_back(it->second);
            // 在这里不可以直接DealExceptionExpired，内部会调用ClearSession,导致迭代器失效
            continue;
        }
        count++;
    }

    auto &cfg = ConfigManager::GetInstance();
    auto self_name = cfg["SelfServer"]["name"];
    auto count_str = std::to_string(count);

    RedisManager::GetInstance()->HSet(LOGIN_COUNT_PREFIX, self_name, count_str);

    for (const auto &session : expired_sessions) {
        session->DealExceptionSession();
    }

    _timer.expires_after(std::chrono::seconds(30));
    _timer.async_wait(
        [this](const boost::system::error_code &ec) { on_timer(ec); });
}

void Server::StopTimer() { _timer.cancel(); }
void Server::StartTimer() {
    _timer.async_wait(
        [self = shared_from_this()](const boost::system::error_code &ec) {
            self->on_timer(ec);
        });
    _timer_running = true;
}
