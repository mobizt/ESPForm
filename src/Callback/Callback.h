
#ifndef Callback_H
#define Callback_H

#ifdef ESP8266

#include <functional>
#include <Schedule.h>
#include <ets_sys.h>

class Callback
{
public:
    Callback();
    ~Callback();

    typedef void (*T_Callback)(void *);
    typedef std::function<void(void)> callback_function_t;

    template <typename T>
    void set_callback(void (*callback)(T), T arg)
    {
       // static_assert(sizeof(T) <= sizeof(void *), "attach() callback argument size must be <= sizeof(void*)");
        _set_callback(reinterpret_cast<T_Callback>(callback), (void *)arg);
    }

    void run_callback(){
        _run_callback();
    }

    void set_scheduled_callback(callback_function_t callback)
    {
        _callback_function = std::move([callback]() { schedule_function(callback); });
        _callback_function();
    }
    

    T_Callback _cb;
    void *_arg;

protected:
    void _set_callback(T_Callback callback, void *arg);
    void _run_callback();

    callback_function_t _callback_function = nullptr;

};

#endif

#endif