#ifndef RANDOM_STREAM_H
#define RANDOM_STREAM_H

class RandomStream
{
public:
    RandomStream(unsigned int seed = 0);
    void SetValue(unsigned int v);
    unsigned int NextValue();

    static const unsigned int MaxValue;

private:
    unsigned long long m_x;

};

#endif