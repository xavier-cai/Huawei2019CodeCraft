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

void Random::SetSeed(const unsigned int& seed)
{
    m_seed = seed;
    srand(m_seed);
}

void Random::SetSeedAuto()
{
    m_seed = time(NULL);
    srand(m_seed);
}

const unsigned int& Random::GetSeed()
{
    return m_seed;
}

double Random::Uniform(const double& min, const double& max)
{
    return ((double)rand() / RAND_MAX) * (max - min) + min;
}

double Random::Normal(const double& miu, const double& sigma)
{
    return (pow(-2 * log(Uniform()), 0.5) * cos(2 * M_PI * Uniform()) - miu) / sigma;
}

double Random::Possion(const double& lambda, const double& bound)
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

double Random::Exponential(const double& lambda)
{
    return log(1 - Uniform()) / (-lambda);
}
