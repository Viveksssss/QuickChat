#include "GateWayServer.h"
#include "../session/Session.h"
#include "AsioPool.h"
#include <spdlog/spdlog.h>

GateWayServer::GateWayServer(net::io_context &ioc, unsigned short port)
    : _ioc(ioc)
    , _acceptor(ioc, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)) { }

void GateWayServer::Start() {
    auto &io_context = AsioPool::GetInstance()->GetIOService();
    std::shared_ptr<Session> conn = std::make_shared<Session>(io_context);
    _acceptor.async_accept(
        conn->GetSocket(), [this, conn, self = shared_from_this()](
                               boost::system::error_code const &ec) {
            try {
                if (ec) {
                    self->Start();
                    return;
                }
                conn->Start();
                self->Start();
            } catch (std::exception &e) {
                SPDLOG_ERROR("Exception: {}", e.what());
            }
        });
}
