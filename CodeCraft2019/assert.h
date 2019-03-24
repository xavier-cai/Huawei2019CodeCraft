#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <iostream>
#include <exception>
#include <cstdlib>

#define ASSERT_ON

#ifdef ASSERT_ON

#define ASSERT_MSG(condition, message)             \
  do                                                  \
    {                                                 \
      if (!(condition))                               \
        {                                             \
          std::cerr << "assert failed. cond=\"" <<    \
          # condition << "\", " << message  \
          << " file=" << __FILE__ << ", line=" <<    \
          __LINE__ << "\n";                     \
          std::terminate();                     \
        }                                             \
    }                                                 \
  while (false)
#else
#define ASSERT_MSG(condition, message) condition
#endif

#define ASSERT(condition) ASSERT_MSG(condition, "")

#endif // MY_ASSERT_H
