// MTMultiplexTest.h : Include file for standard system include files,
// or project specific include files.
#ifndef MTMULTIPLEXTEST_H
#define MTMULTIPLEXTEST_H

#include <iostream>

#ifdef __MINGW32__
    #ifdef __linux__
        #include "mingw_stdthreads/mingw.thread.h"
    #else
        #include <thread>
    #endif
#else
    #include <thread>
#endif

#include <string>

#endif