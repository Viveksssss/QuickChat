#ifndef SINGLETON_H
#define SINGLETON_H

#include "global.h"

template<typename T>
class Singleton{
protected:
    Singleton()=default;
    ~Singleton() = default;
public:
    Singleton(const Singleton<T>&)=delete;
    Singleton&operator=(const Singleton<T>&)=delete;
    static std::shared_ptr<T> GetInstance(){
        static std::shared_ptr<T> _instance = std::shared_ptr<T>(new T);
        return _instance;
    }

};

#endif
