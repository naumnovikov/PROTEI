#ifndef MSISDNGENERATOR_H
#define MSISDNGENERATOR_H

#include <random>

#include "common_types.h"

class MSISDNGenerator {
 public:
  static inline MSISDN generate() {
    // std::mt19937 is not thread-safe
    // so I used thread_local here
    // https://stackoverflow.com/questions/77377046/is-a-stdmt19937-static-function-variable-thread-safe

    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(1000000, 9999999);
    // Generating a Russian number, but it can be any
    return "7999" + std::to_string(distribution(generator));
  }
};

#endif  // MSISDNGENERATOR_H