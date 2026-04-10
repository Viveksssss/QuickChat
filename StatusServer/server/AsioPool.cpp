#include "AsioPool.h"
#include <iostream>
#include <memory>

AsioPool::AsioPool(std::size_t size)
    : _services(size)
    , _works()
    , _next_services(0)
{
    _works.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        _works.emplace_back(std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(_services[i])));
    }
    _threads.reserve(size);
    for (std::size_t i = 0; i < size; i++) {
        _threads.emplace_back([this, i] {
            _services[i].run();
        });
    }
}

boost::asio::io_context& AsioPool::GetIOService()
{
    auto& service = _services[_next_services];
    _next_services = (_next_services + 1) % _services.size();
    return service;
}

void AsioPool::Stop()
{
    for (auto& work : _works) {
        work.reset();
    }
    for (auto& thread : _threads) {
        thread.join();
    }
}

AsioPool::~AsioPool()
{
    Stop();
    //std::cout << "The AsioPool Destruct" << std::endl;
}
