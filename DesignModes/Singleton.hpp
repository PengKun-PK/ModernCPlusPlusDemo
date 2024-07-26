#pragma once

template <typename T>
class Singleton
{
public:
    static T& s_getInstance()
    {
        static T s_oT;
        return s_oT;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;

};

template <typename T>
T& Instance()
{
    return Singleton<T>::s_getInstance();
}