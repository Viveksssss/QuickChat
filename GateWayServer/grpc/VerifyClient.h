#ifndef VERIFYCLIENT_H
#define VERIFYCLIENT_H

#include "../global/ConfigManager.h"
#include "../global/Singleton.h"
#include "../global/const.h"
#include "RPCPool.h"

using message::GetSecurityCodeRequest;
using message::GetSecurityCodeResponse;
using message::VarifyService;

class VerifyClient : public Singleton<VerifyClient> {
    friend class Singleton<VerifyClient>;

public:
    GetSecurityCodeResponse GetSecurityCode(const std::string& email);

private:
    VerifyClient();

    std::unique_ptr<RPCPool<VarifyService, VarifyService::Stub>> _pool;
};

#endif