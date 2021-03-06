/*
 * FEASST - Free Energy and Advanced Sampling Simulation Toolkit
 * http://pages.nist.gov/feasst, National Institute of Standards and Technology
 * Harold W. Hatch, harold.hatch@nist.gov
 *
 * Permission to use this data/software is contingent upon your acceptance of
 * the terms of LICENSE.txt and upon your providing
 * appropriate acknowledgments of NIST's creation of the data/software.
 */

#include "random_nr3.h"

#ifdef FEASST_NAMESPACE_
namespace feasst {
#endif  // FEASST_NAMESPACE_

RandomNR3::RandomNR3(const unsigned long long iseed
  ) : Random(iseed) {
  verbose_ = 0;
  className_.assign("RandomNR3");
  seed(iseed);
}

RandomNR3::RandomNR3(const char* fileName
  ) : Random(fileName) {
  verbose_ = 0;
  className_.assign("RandomNR3");
  u_ = fstoull("u", fileName);
  v_ = fstoull("v", fileName);
  w_ = fstoull("w", fileName);
}

void RandomNR3::seed(const unsigned long long iseed) {
  Random::seed(iseed);
  v_ = 4101842887655102017LL;
  w_ = 1;
  u_ = seed_ ^ v_;
  int64();
  v_ = u_;
  int64();
  w_ = v_;
  int64();
}

double RandomNR3::uniform() {
  return 5.42101086242752217E-20 * int64();
}

unsigned long long RandomNR3::int64() {
  //cout << "u " << u_ << " v " << v_ << " w " << w_ << endl;
  u_ = u_*2862933555777941757LL + 7046029254386353087LL;
  v_ ^= v_ >> 17; v_ ^= v_ << 31; v_ ^= v_ >> 8;
  w_ = 4294957665U*(w_ & 0xffffffff) + (w_ >> 32);
  unsigned long long x = u_ ^ (u_ << 21); x ^= x >> 35; x ^= x << 4;
  //cout << "u " << u_ << " v " << v_ << " w " << w_ << endl;
  return (x + v_) ^ w_;
}

void RandomNR3::writeRestart(const char* fileName) {
  fileBackUp(fileName);
  std::ofstream file(fileName);
  file << "# u " << u_ << endl;
  file << "# v " << v_ << endl;
  file << "# w " << w_ << endl;
}

#ifdef FEASST_NAMESPACE_
}  // namespace feasst
#endif  // FEASST_NAMESPACE_


