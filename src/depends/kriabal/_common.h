#ifndef _common_h__
#define _common_h__

#include <exception>

namespace kriabal
{
    class RuntimeError : public std::exception {};
}

#endif
