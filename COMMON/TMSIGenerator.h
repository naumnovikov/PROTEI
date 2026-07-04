#ifndef TMSIGENERATOR_H
#define TMSIGENERATOR_H

#include <climits>
#include <random>

#include "common_types.h"

class TMSIGenerator {
 public:
  static inline TMSI generate() {
    // std::mt19937 is not thread-safe
    // so I used thread_local here
    // https://stackoverflow.com/questions/77377046/is-a-stdmt19937-static-function-variable-thread-safe

    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<uint32_t> distribution(1, UINT32_MAX);
    return distribution(generator);
  }
};

#endif  // TMSIGENERATOR_H