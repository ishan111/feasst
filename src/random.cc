/**
 * \file
 *
 * \brief trial moves for Monte Carlo
 */

#include "random.h"

/**
 * Constructor
 */
Random::Random(const unsigned long long iseed
  ) : seed_(iseed) {
  verbose_ = 0;
  className_.assign("Random");
}
Random::Random(const char* fileName) {
  verbose_ = 0;
  className_.assign("Random");
  (void) fileName;
}

/**
 * seed random number generator
 */
void Random::seed(const unsigned long long iseed) {
  seed_ = iseed;
  //mout_("note", std::ostringstream().flush() << "time(seed): " << seed_);
}


