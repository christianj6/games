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

// template definitions live in the header — required for instantiation at
// the use site
template <typename T>
RandomNumberGenerator<T>::RandomNumberGenerator(T start, T end)
    : distribution(start, end), generator(std::random_device{}()) {}

template <typename T> T RandomNumberGenerator<T>::operator()() {
  return distribution(generator);
}
