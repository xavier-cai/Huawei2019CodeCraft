#ifndef RANDOM_H
#define RANDOM_H

class Random {
private:
    Random();
    static unsigned int m_seed;

public:
    static void SetSeed(const unsigned int& seed);
    static void SetSeedAuto();
    static const unsigned int& GetSeed();
    
    static double Uniform(const double& min = 0.0, const double& max = 1.0);
    static double Normal(const double& miu = 0.0, const double& sigma = 1.0);
    static double Possion(const double& lambda = 1.0, const double& bound = 0.0); //bound <= 0 means no bound
    static double Exponential(const double& lambda = 1.0);
    
};//class Random

#endif
