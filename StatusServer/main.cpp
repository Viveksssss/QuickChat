#include "global/ConfigManager.h"
#include "grpc/StatusServiceImpl.h"
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <grpc++/grpc++.h>
#include <spdlog/spdlog.h>
#include <thread>

void RunServer() {
    auto &cfg = ConfigManager::GetInstance();
    // 路径
    std::string server_address(cfg["StatusServer"]["host"] + ":" +
                               cfg["StatusServer"]["port"]);
    // 配置和构建 gRPC 服务器的核心类
    grpc::ServerBuilder builder;
    // 设置端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // 注册服务
    StatusServiceImpl service;
    builder.RegisterService(&service);
    // 构建grpc服务器并启动
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    // 创建boost.asio的io_context
    boost::asio::io_context io_context;
    // 捕获退出
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    // 设置异步等待退出信号
    signals.async_wait(
        [&](const boost::system::error_code &error, int signal_number) {
            if (!error) {
                server->Shutdown();
                io_context.stop();
            }
        });
    // ddd单独的线程运行io_context
    std::jthread([&io_context]() { io_context.run(); });
    // 等待服务器的关闭
    server->Wait();
}

int main() {
    try {
        SPDLOG_INFO("Starting StatusServer");
        RunServer();
    } catch (const std::exception &e) {
        spdlog::error("Exception: {}", e.what());
    }
    return 0;
}
