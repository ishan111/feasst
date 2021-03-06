/*
 * FEASST - Free Energy and Advanced Sampling Simulation Toolkit
 * http://pages.nist.gov/feasst, National Institute of Standards and Technology
 * Harold W. Hatch, harold.hatch@nist.gov
 *
 * Permission to use this data/software is contingent upon your acceptance of
 * the terms of LICENSE.txt and upon your providing
 * appropriate acknowledgments of NIST's creation of the data/software.
 */

#include "./pair_soft_sphere.h"

#ifdef FEASST_NAMESPACE_
namespace feasst {
#endif  // FEASST_NAMESPACE_

PairSoftSphere::PairSoftSphere(Space* space, const double rCut)
  : Pair(space, rCut) {
  defaultConstruction_();
}

PairSoftSphere::PairSoftSphere(Space* space, const char* fileName)
  : Pair(space, fileName) {
  defaultConstruction_();
  string str = fstos("npotentialparameter", fileName);
  if (!str.empty()) {
    initExponent(stoi(str));
  }
}

void PairSoftSphere::defaultConstruction_() {
  className_.assign("PairSoftSphere");
  initExponent();
}

void PairSoftSphere::writeRestart(const char* fileName) {
  writeRestartBase(fileName);
  std::ofstream file(fileName, std::ios_base::app);
  file << "# npotentialparameter " << n_ << endl;
}

void PairSoftSphere::multiPartEnerAtomCutInner(const double &r2,
  const int &itype,
  const int &jtype) {
  const double sigij = sigij_[itype][jtype];
  peTot_ += pow(sigij*sigij/r2, n_/2.);
}

double PairSoftSphere::b2reduced() {
  ASSERT(n_ == 12, "virial coefficient hard coded for n=12 for gamma[1-3/n]");
  const double gamma = 1.225416702465178;
  const double sigij = sigij_[0][0];
  return 2.*PI/3.*pow(sigij, 3)*gamma;
}

PairSoftSphere* PairSoftSphere::clone(Space* space) const {
  PairSoftSphere* p = new PairSoftSphere(*this);
  p->reconstruct(space);
  return p;
}

shared_ptr<PairSoftSphere> makePairSoftSphere(Space* space, const double rCut) {
  return make_shared<PairSoftSphere>(space, rCut);
}

#ifdef FEASST_NAMESPACE_
}  // namespace feasst
#endif  // FEASST_NAMESPACE_

