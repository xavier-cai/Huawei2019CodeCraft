#ifndef RANDOM_H
#define RANDOM_H

class Random {
private:
    Random();
    static unsigned int m_seed;

public:
    static void SetSeed(unsigned int seed);
    static void SetSeedAuto();
    static unsigned int GetSeed();
    
    static double Uniform(double min = 0.0, double max = 1.0);
    static double Normal(double miu = 0.0, double sigma = 1.0);
    static double Possion(double lambda = 1.0, double bound = 0.0); //bound <= 0 means no bound
    static double Exponential(double lambda = 1.0);
    
};//class Random

#endif
