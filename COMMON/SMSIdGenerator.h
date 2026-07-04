#ifndef SMSIDGENERATOR_H
#define SMSIDGENERATOR_H

#include <climits>
#include <random>

#include "common_types.h"

class SMSIdGenerator {
 public:
  static inline SMS_ID generate() {
    // std::mt19937 is not thread-safe
    // so I used thread_local here
    // https://stackoverflow.com/questions/77377046/is-a-stdmt19937-static-function-variable-thread-safe

    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(1, INT_MAX);
    return distribution(generator);
  }
};

#endif  // SMSIDGENERATOR_H