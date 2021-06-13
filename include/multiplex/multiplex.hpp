/// @file MTMultiplex.h
/// @brief This file contains general library exports.
#ifndef MULTIPLEX_H
#define MULTIPLEX_H

// Library export macro
#if defined(_MSC_VER)
//  Microsoft
#define MULTIPLEX_EXPORT __declspec(dllexport)
#define MULTIPLEX_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define MULTIPLEX_EXPORT __attribute__((visibility("default")))
#define MULTIPLEX_IMPORT
#else
//  do nothing and hope for the best?
#define MULTIPLEX_EXPORT
#define MULTIPLEX_IMPORT
#pragma warning Unknown dynamic link import / export semantics.
#endif

#include <exception>
#include <string>
#include "types.hpp"

#define MULTIPLEX_MAX_CHANNELS 32 /* The amount of instance channels. */
#define MULTIPLEX_MAX_SERVER_CONNECTIONS 1024
#define MULTIPLEX_MAX_DATA_SIZE 65535

class MultiplexException : public std::exception {
public:
    MultiplexException(const char *what)
    {
        error = what;
    }

    MultiplexException(std::string what)
    {
        error = what.c_str();
    }

    virtual const char *what() const throw()
    {
        return error;
    }

private:
    const char *error;
};
#endif
