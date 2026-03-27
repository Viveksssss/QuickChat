#include "const.h"

Defer::Defer(std::function<void()> func)
    : m_func(func)
{
}
Defer::~Defer()
{
    m_func();
}
