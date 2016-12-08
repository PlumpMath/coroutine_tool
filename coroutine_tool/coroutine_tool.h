#pragma once

//#if defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
# if !defined(_WIN32_WINNT) && !defined(_WIN32_WINDOWS)
#  if defined(_MSC_VER) || defined(__BORLANDC__)
#   define _WIN32_WINNT 0x0501
#  endif
# endif
//#endif

#include "coroutine_env.h"

namespace coroutine_tool
{

}
