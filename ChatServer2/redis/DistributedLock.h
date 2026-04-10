#ifndef DISTRIBUTEDLOCK_H
#define DISTRIBUTEDLOCK_H

#include <string>
#include <thread>

struct redisContext;
class DistributedLock {
private:
    DistributedLock() = default;

public:
    ~DistributedLock() = default;
    /**
     * @brief  得到实例
     *
     * @return DistributedLock&
     */
    static DistributedLock& GetInstance();
    /**
     * @brief 在redis上获取分布式锁
     *
     * @param context
     * @param keyname
     * @param timeout
     * @param acquireTimeout
     * @return std::string
     */
    std::string AcquireLock(redisContext* context, const std::string& keyname, int timeout,
        int acquireTimeout);
    /**
     * @brief 释放分布式锁
     *
     * @param key
     * @param identifier
     */
    bool ReleaseLock(redisContext* context, const std::string& keyname, const std::string& identifier);
    std::string GenerateUUID();
};

#endif