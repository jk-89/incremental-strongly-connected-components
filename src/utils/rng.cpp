#include "rng.hpp"

#include <chrono>

RNG::RNG() {
    const auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    engine.seed(static_cast<uint64_t>(now));
}

RNG &RNG::instance() {
    static RNG rng_instance;
    return rng_instance;
}

void RNG::seed(uint32_t seed) { engine.seed(seed); }

int RNG::randint(int min, int max) {
    std::uniform_int_distribution dist(min, max);
    return dist(engine);
}

int RNG::randint() { return randint(0, INT32_MAX); }

double RNG::randreal(double min, double max) {
    std::uniform_real_distribution dist(min, max);
    return dist(engine);
}
