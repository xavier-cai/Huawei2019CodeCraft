#include "random-stream.h"

#define RANDOM_A 214013L
#define RANDOM_C 2531011L
#define RANDOM_M 0x7fff

const unsigned int RandomStream::MaxValue(RANDOM_M);

RandomStream::RandomStream(unsigned int seed)
    : m_x(seed % MaxValue)
{ }

void RandomStream::SetValue(unsigned int v)
{
    m_x = v % MaxValue;
}

unsigned int RandomStream::NextValue()
{
    return (m_x = ((m_x * RANDOM_A + RANDOM_C) >> 16) & RANDOM_M);
}

#undef RANDOM_A
#undef RANDOM_C
#undef RANDOM_M
