#include "random.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Random::Random()
{ }

unsigned int Random::m_seed = 0;

void Random::SetSeed(unsigned int seed)
{
    m_seed = seed;
    srand(m_seed);
}

void Random::SetSeedAuto()
{
    m_seed = time(NULL);
    srand(m_seed);
}

unsigned int Random::GetSeed()
{
    return m_seed;
}

double Random::Uniform(double min, double max)
{
    return (double(rand()) / RAND_MAX) * (max - min) + min;
}

double Random::Normal(double miu, double sigma)
{
    return (pow(-2 * log(Uniform()), 0.5) * cos(2 * M_PI * Uniform()) - miu) / sigma;
}

double Random::Possion(double lambda, double bound)
{
    while(true)
    {
        double v = -lambda * log(Uniform());
        if(bound <= 0 || v < bound)
        {
            return v;
        }
    }
    return 0;
}

double Random::Exponential(double lambda)
{
    return log(1 - Uniform()) / (-lambda);
}
