#ifndef GATEWAYSERVER_H
#define GATEWAYSERVER_H

#include "../global/const.h"
#include <memory>
<<<<<<< HEAD
namespace net = boost::asio;

class GateWayServer : public std::enable_shared_from_this<GateWayServer> {
public:
    GateWayServer(net::io_context &ioc, unsigned short port);
=======

class GateWayServer : public std::enable_shared_from_this<GateWayServer> {
public:
    GateWayServer(net::io_context& ioc, unsigned short port);
>>>>>>> origin/main
    void Start();

private:
    net::ip::tcp::acceptor _acceptor;
<<<<<<< HEAD
    net::io_context &_ioc;
};

#endif
=======
    net::io_context& _ioc;
};

#endif
>>>>>>> origin/main
