#include "signalrouter.h"

SignalRouter::SignalRouter(QObject *parent)
    : QObject{parent}
{}

SignalRouter &SignalRouter::GetInstance()
{
    static SignalRouter instance;
    return instance;
}
