#ifndef RNG_HPP
#define RNG_HPP

#include <random>

// Singleton-style random number generator class.
class RNG {
   public:
    static RNG &instance();

    void seed(uint32_t seed);

    int randint(int min, int max);
    int randint();
    double randreal(double min, double max);

   private:
    RNG();
    std::mt19937 engine;
};

#endif  // RNG_HPP
