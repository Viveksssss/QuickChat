#include "./global/ConfigManager.h"
#include "./server/GateWayServer.h"
#include <boost/asio.hpp>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
using json = nlohmann::json;

int main(int, char **) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
    spdlog::set_level(spdlog::level::debug);
    auto &cfgMgr = ConfigManager::GetInstance();

    try {
        unsigned short port = 9999;
        net::io_context ioc;
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait(
            [&ioc](const boost::system::error_code &, int) { ioc.stop(); });
        std::make_shared<GateWayServer>(ioc, port)->Start();
        SPDLOG_INFO("GateWayServer starting on port: {}",
                    cfgMgr["GateWayServer"]["port"]);
        ioc.run();

    } catch (std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
        return 1;
    }
}
