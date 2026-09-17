#pragma once
#include <random>
#include <type_traits>

template <typename T> class RandomNumberGenerator {
public:
  RandomNumberGenerator(T, T);
  T operator()();

private:
  // conditionally define the distribution type based on T
  using Distribution = std::conditional_t<std::is_floating_point_v<T>,
                                          std::uniform_real_distribution<T>,
                                          std::uniform_int_distribution<T>>;
  Distribution distribution;
  std::mt19937 generator;
};

// template class definitions must be in the .h file bc of compiler ish
template <typename T>
RandomNumberGenerator<T>::RandomNumberGenerator(T start, T end)
    : generator(std::random_device{}()) {
  distribution = Distribution(start, end);
}

template <typename T> T RandomNumberGenerator<T>::operator()() {
  return distribution(generator);
}
