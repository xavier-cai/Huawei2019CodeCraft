#ifndef RANDOM_H
#define RANDOM_H

#include "random-stream.h"

class Random {
private:
    static Random Instance;
    unsigned int m_seed;
    RandomStream m_stream;

public:
    Random(unsigned int seed = 0);

    void SetSeedImpl(const unsigned int& seed);
    void SetSeedAutoImpl();
    const unsigned int& GetSeedImpl() const;

    double NextUniform(const double& min = 0.0, const double& max = 1.0);
    double NextNormal(const double& miu = 0.0, const double& sigma = 1.0);
    double NextPossion(const double& lambda = 1.0, const double& bound = 0.0); //bound <= 0 means no bound
    double NextExponential(const double& lambda = 1.0);

    /* static functions */
    static void SetSeed(const unsigned int& seed);
    static void SetSeedAuto();
    static const unsigned int& GetSeed();
    
    static double Uniform(const double& min = 0.0, const double& max = 1.0);
    static double Normal(const double& miu = 0.0, const double& sigma = 1.0);
    static double Possion(const double& lambda = 1.0, const double& bound = 0.0); //bound <= 0 means no bound
    static double Exponential(const double& lambda = 1.0);
    
};//class Random

#endif
