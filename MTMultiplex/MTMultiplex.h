// MTMultiplex.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#ifndef MTMULTIPLEX_EXPORT_H
#define MTMULTIPLEX_EXPORT_H

#ifdef MTMULTIPLEX_STATIC_DEFINE
#  define MTMULTIPLEX_EXPORT
#  define MTMULTIPLEX_NO_EXPORT
#else
#  ifndef MTMULTIPLEX_EXPORT
#    ifdef MTMultiplex_EXPORTS
        /* We are building this library */
#      define MTMULTIPLEX_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define MTMULTIPLEX_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef MTMULTIPLEX_NO_EXPORT
#    define MTMULTIPLEX_NO_EXPORT 
#  endif
#endif

#ifndef MTMULTIPLEX_DEPRECATED
#  define MTMULTIPLEX_DEPRECATED __declspec(deprecated)
#endif

#ifndef MTMULTIPLEX_DEPRECATED_EXPORT
#  define MTMULTIPLEX_DEPRECATED_EXPORT MTMULTIPLEX_EXPORT MTMULTIPLEX_DEPRECATED
#endif

#ifndef MTMULTIPLEX_DEPRECATED_NO_EXPORT
#  define MTMULTIPLEX_DEPRECATED_NO_EXPORT MTMULTIPLEX_NO_EXPORT MTMULTIPLEX_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MTMULTIPLEX_NO_DEPRECATED
#    define MTMULTIPLEX_NO_DEPRECATED
#  endif
#endif

#endif /* MTMULTIPLEX_EXPORT_H */

#include <iostream>
#include <map>
#include <string>

enum MultiplexActions {
    EditChannel,
    Server
};

MTMULTIPLEX_EXPORT typedef struct MultiplexInstanceUser {
    unsigned int channel;
    void* peer;
};

MTMULTIPLEX_EXPORT typedef struct MultiplexInstance {
    unsigned int id = 0;
    char* info;
    std::map<int, MultiplexInstanceUser> users;

};


MTMULTIPLEX_EXPORT typedef struct MultiplexUser {
    unsigned int userId;
    int channelInstances[32];
};

MTMULTIPLEX_EXPORT int Init_ENet();

class MTMULTIPLEX_EXPORT Multiplex
{
public:
    Multiplex();
    ~Multiplex();
    int Multiplex::Client_Connect(char* host_name, int port);
    int Multiplex::Setup_Host(bool is_server, char* host_name, int port);
    int Multiplex::Process_Event();
    int Multiplex::Process_Server_Event();
    int Multiplex::Send(const char* data, unsigned int dataLength, unsigned int channel, bool reliable);

private:
    void* peer;
    void* client;
    unsigned int clientIdIncrement = 0;
    std::map<int, MultiplexInstance> Instances;
};
