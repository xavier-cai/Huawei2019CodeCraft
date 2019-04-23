#include "random.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "assert.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Random Random::Instance;

Random::Random(unsigned int seed)
    : m_seed(seed), m_stream(seed)
{ }

void Random::SetSeedImpl(const unsigned int& seed)
{
    m_seed = seed;
    srand(m_seed);
    m_stream.SetValue(seed);
}

void Random::SetSeedAutoImpl()
{
    SetSeed(time(NULL));
}

const unsigned int& Random::GetSeedImpl() const
{
    return m_seed;
}

double Random::NextUniform(const double& min, const double& max)
{
    //return ((double)rand() / RAND_MAX) * (max - min) + min;
    return ((double)m_stream.NextValue() / RandomStream::MaxValue) * (max - min) + min;
}

double Random::NextNormal(const double& miu, const double& sigma)
{
    return pow(-2 * log(Uniform()), 0.5) * cos(2 * M_PI * Uniform()) * sigma + miu;
}

double Random::NextPossion(const double& lambda, const double& bound)
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

double Random::NextExponential(const double& lambda)
{
    return log(1 - Uniform()) / (-lambda);
}

void Random::SetSeed(const unsigned int& seed)
{
    Instance.SetSeedImpl(seed);
}

void Random::SetSeedAuto()
{
    Instance.SetSeedAutoImpl();
}

const unsigned int& Random::GetSeed()
{
    return Instance.GetSeedImpl();
}
    
double Random::Uniform(const double& min, const double& max)
{
    return Instance.NextUniform(min, max);
}

double Random::Normal(const double& miu, const double& sigma)
{
    return Instance.NextNormal(miu, sigma);
}

double Random::Possion(const double& lambda, const double& bound)
{
    return Instance.NextPossion(lambda, bound);
}

double Random::Exponential(const double& lambda)
{
    return Instance.NextExponential(lambda);
}
