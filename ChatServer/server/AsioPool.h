#ifndef ASIOPOOL_H
#define ASIOPOOL_H

#include "../global/Singleton.h"
#include <boost/asio.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <thread>
#include <vector>

class AsioPool : public Singleton<AsioPool> {
    friend Singleton<AsioPool>;

public:
    ~AsioPool();
    AsioPool(const AsioPool&) = delete;
    AsioPool& operator=(const AsioPool&) = delete;

    boost::asio::io_context& GetIOService();
    void Stop();

private:
    AsioPool(std::size_t size = std::thread::hardware_concurrency());

private:
    std::vector<boost::asio::io_context> _services;
    std::vector<std::thread> _threads;
    std::vector<std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>> _works;
    std::size_t _next_services;
};

#endif