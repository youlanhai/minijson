#pragma once

#ifndef SJ_USE_LARGE_NUMBER
#define SJ_USE_LARGE_NUMBER 1
#endif

#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define SJ_PLATFORM_64BIT 1
#else
#define SJ_PLATFORM_64BIT 0
#endif

#ifndef SJ_ASSERT
#   if ((defined DEBUG) || defined(_DEBUG))
#include <cassert>
#       define SJ_ASSERT(EXP) assert(EXP)
#   else
#       define SJ_ASSERT(EXP)
#   endif
#endif //SJ_ASSERT

#define SJ_DISABLE_COPY_ASSIGN(CLASS) \
    CLASS(const CLASS&) = delete; \
    const CLASS& operator=(const CLASS&) = delete

#define NS_SMARTJSON_BEGIN  namespace smartjson {
#define NS_SMARTJSON_END    }
#define NS_SMARTJSON        smartjson::
#define USING_NS_SMARTJSON  using namespace smartjson;
