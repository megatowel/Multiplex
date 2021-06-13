#ifndef MULTIPLEX_ERROR_HPP
#define MULTIPLEX_ERROR_HPP
#include "multiplex/multiplex.hpp"

#define MULTIPLEX_ERROR(E) throw MultiplexException(std::string("MULTIPLEX RUNTIME ERROR: ") + #E + "(at " + __func__ + " in " + __FILE__ + ":" + std::to_string(__LINE__) + ")\n")
#endif