

#ifndef Callback_CPP
#define Callback_CPP

#ifdef ESP8266

#include "c_types.h"
#include "eagle_soc.h"
#include "osapi.h"

#include "Callback.h"



Callback::Callback()
{
}

Callback::~Callback()
{
    
}

void Callback::_set_callback(T_Callback callback, void *arg)
{
    _cb = callback;
    _arg = arg;
   
}

void Callback::_run_callback()
{
    _cb(_arg);
}

#endif

#endif