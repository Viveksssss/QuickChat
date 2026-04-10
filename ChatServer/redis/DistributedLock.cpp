#include "DistributedLock.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <hiredis/hiredis.h>

using namespace std::chrono_literals;

DistributedLock& DistributedLock::GetInstance()
{
    static DistributedLock instance;
    return instance;
}

std::string DistributedLock::AcquireLock(redisContext* context, const std::string& keyname, int timeout, int acquireTimeout)
{
    std::string idenetifier = GenerateUUID();
    std::string key = "lock_" + keyname;
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::seconds(acquireTimeout);
    while (std::chrono::steady_clock::now() < end) {
        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d", key.c_str(), idenetifier.c_str(), timeout);
        if (reply != nullptr) {
            if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
                freeReplyObject(reply);
                return idenetifier;
            }
            freeReplyObject(reply);
        }
        std::this_thread::sleep_for(5ms);
    }
    return "";
}

/**
 * 释放分布式锁使用了lua脚本：
 *
 * if redis.call("get",KEYS[1]) == ARGV[1] then
 *   return redis.call("del",KEYS[1])
 * else
 *   return 0
 * end
 *
 */
bool DistributedLock::ReleaseLock(redisContext* context, const std::string& keyname, const std::string& identifier)
{
    std::string key = "lock_" + keyname;
    const char* luaScript = R"(
        if redis.call("get",KEYS[1]) == ARGV[1] then
            return redis.call("del",KEYS[1])
        else
            return 0
        end
    )";

    redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s", luaScript, key.c_str(), identifier.c_str());
    bool success = false;
    if (reply != nullptr) {
        if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
            success = true;
        }
        freeReplyObject(reply);
    }
    return success;
}

std::string DistributedLock::GenerateUUID()
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return to_string(uuid);
}
