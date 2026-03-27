#ifndef GATEWAYSERVER_H
#define GATEWAYSERVER_H

#include "../global/const.h"
#include <memory>

class GateWayServer : public std::enable_shared_from_this<GateWayServer> {
public:
    GateWayServer(net::io_context& ioc, unsigned short port);
    void Start();

private:
    net::ip::tcp::acceptor _acceptor;
    net::io_context& _ioc;
};

#endif