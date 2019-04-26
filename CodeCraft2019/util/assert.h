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
          throw int(0);                     \
        }                                             \
    }                                                 \
  while (false)
#else
#define ASSERT_MSG(...)
#endif

#define ASSERT(condition) ASSERT_MSG(condition, "")

#endif // MY_ASSERT_H
