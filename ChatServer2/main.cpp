#include "global/ConfigManager.h"
#include "global/const.h"
#include "grpc/ChatGrpcServer.h"
#include "redis/RedisManager.h"
#include "server/AsioPool.h"
#include "server/LogicSystem.h"
#include "server/Server.h"

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/beast/http/field.hpp>
#include <grpc++/grpc++.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>
#include <spdlog/spdlog.h>
#include <thread>

int main()
{

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
    spdlog::set_level(spdlog::level::debug);

    auto& cfg = ConfigManager::GetInstance();
    auto server_name = cfg["SelfServer"]["name"];
    SPDLOG_INFO("开始创建 ChatGrpcServer 实例...");

    {
        RedisManager::GetInstance()->InitCount(server_name);
        Defer defer([server_name]{
            RedisManager::GetInstance()->DelCount(server_name);
            RedisManager::GetInstance()->Close();
        });

        std::string server_address = cfg["SelfServer"]["host"] + ":" + cfg["SelfServer"]["RPCPort"];
        SPDLOG_INFO("开始创建 ChatGrpcServer 实例...");
        ChatGrpcServer service;
        SPDLOG_INFO("ChatGrpcServer 实例创建完成，地址: {}", (void*)&service);
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        SPDLOG_INFO("Grpc Server On: {}", server_address);

        std::thread grpc_server([&server]() {
            server->Wait();
        });

        auto pool = AsioPool::GetInstance();
        boost::asio::io_context ioc;

        auto port = cfg["SelfServer"]["port"];
        auto server_ptr = std::make_shared<Server>(ioc, std::stoi(port));
        LogicSystem::GetInstance()->SetServer(server_ptr);
        service.SetServer(server_ptr);

        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, pool, &server](const boost::system::error_code& /*error*/, int /*signal_number*/) {
            pool->Stop();
            ioc.stop();
            server->Shutdown();
        });

        server_ptr->Start();
        ioc.run();
        grpc_server.join();
    }
    return 0;
}
