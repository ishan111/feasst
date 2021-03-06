/*
 * FEASST - Free Energy and Advanced Sampling Simulation Toolkit
 * http://pages.nist.gov/feasst, National Institute of Standards and Technology
 * Harold W. Hatch, harold.hatch@nist.gov
 *
 * Permission to use this data/software is contingent upon your acceptance of
 * the terms of LICENSE.txt and upon your providing
 * appropriate acknowledgments of NIST's creation of the data/software.
 */

#include "pair_hybrid.h"

#ifdef FEASST_NAMESPACE_
namespace feasst {
#endif  // FEASST_NAMESPACE_

PairHybrid::PairHybrid(Space* space, const double rCut)
  : Pair(space, rCut) {
  defaultConstruction_();
}

PairHybrid::PairHybrid(Space* space, const char* fileName)
  : Pair(space, fileName) {
  defaultConstruction_();
  clone_ = 1;

  string strtmp = fstos("pairPrint", fileName);
  if (!strtmp.empty()) pairPrint_ = atoi(strtmp.c_str());
  const int npairs = fstoi("pairtypes", fileName);
  for (int i = 0; i < npairs; ++i) {
    stringstream ss;
    ss << "pairRstFileName" << i;
    const string pairfilestr = fstos(ss.str().c_str(), fileName);
    addPair(makePair(space_, pairfilestr.c_str()));
  }

  // read selected pair
  strtmp = fstos("nselectedpairs", fileName);
  if (!strtmp.empty()) {
    selected_.clear();
    for (int i = 0; i < atoi(strtmp.c_str()); ++i) {
      stringstream ss;
      ss << "selectedpair" << i << "i";
      selected_.push_back(fstoi(ss.str().c_str(), fileName));
    }
  }
}

PairHybrid::~PairHybrid() {
  if (clone_ > 0) {
    for (int i = pairVec_.size() - 1; i >= 0; --i) delete pairVec_[i];
  }
}

PairHybrid* PairHybrid::clone(Space* space) const {
  PairHybrid* p = new PairHybrid(*this);
  p->reconstruct(space);
  return p;
}

void PairHybrid::reconstruct(Space* space) {
  ++clone_;

  // clone all pairs, while preserving order of pair types
  for (unsigned int i = 0; i < pairVec_.size(); ++i) {
    pairVec_[i] = pairVec_[i]->clone(space);
  }
  Pair::reconstruct(space);
}

void PairHybrid::defaultConstruction_() {
  className_.assign("PairHybrid");
  pairPrint_ = 0;
  clone_ = 0;
}

void PairHybrid::initEnergy() {
  if (selected_.size() > 0) {
    // compute Forces from selected pair(s)
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pairVec_[iPair]->initEnergy();
    }

  } else {
    // otherwise, compute Forces for all pairs
    for (int iPair = 0; iPair < nPairs(); ++iPair) {
      pairVec_[iPair]->initEnergy();
    }
  }
}

double PairHybrid::multiPartEner(
  const vector<int> mpart,
  const int flag) {
  double pe = 0.;
  if (selected_.size() > 0) {
    // compute from selected pair(s)
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pe += pairVec_[iPair]->multiPartEner(mpart, flag);
    }

  } else {
    // otherwise, compute for all pairs
    for (int iPair = 0; iPair < nPairs(); ++iPair) {
      pe += pairVec_[iPair]->multiPartEner(mpart, flag);
    }
  }
  return pe;
}

double PairHybrid::peTot() {
  double pe = 0.;
  if (selected_.size() > 0) {
    // compute from selected pair(s)
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pe += pairVec_[iPair]->peTot();
    }

  } else {
    // otherwise, compute for all pairs
    for (int iPair = 0; iPair < nPairs(); ++iPair) {
      pe += pairVec_[iPair]->peTot();
    }
  }
  return pe;
}

double PairHybrid::vrTot() {
  double vrTotTemp = 0.;
  for (int i = 0; i < nPairs(); ++i) {
    vrTotTemp += pairVec_[i]->vrTot();
  }
  return vrTotTemp;
}

void PairHybrid::addPart() {
  for (int i = 0; i < nPairs(); ++i) {
    pairVec_[i]->addPart();
  }
}

void PairHybrid::delPart(const int ipart) {
  for (int i = 0; i < nPairs(); ++i) {
    pairVec_[i]->delPart(ipart);
  }
}

void PairHybrid::delPart(const vector<int> mpart) {
  for (int i = 0; i < nPairs(); ++i) {
    pairVec_[i]->delPart(mpart);
  }
}

void PairHybrid::update(
  const vector<int> mpart,    //!< particles involved in move
  const int flag,         //!< type of move
  const char* uptype    //!< description of update type
  ) {
  if (selected_.size() > 0) {
    // compute from selected pair(s)
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pairVec_[iPair]->update(mpart, flag, uptype);
    }
  } else {
    // all pairs
    for (int i = 0; i < nPairs(); ++i) {
      pairVec_[i]->update(mpart, flag, uptype);
    }
  }
}

void PairHybrid::update(const double de) {
  if (selected_.size() > 0) {
    // compute from selected pair(s)
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pairVec_[iPair]->update(de);
    }
  } else {
    // all pairs
    for (int i = 0; i < nPairs(); ++i) {
      // split the energy change evenly among the pairs, which is wrong,
      // but it works when all the energies are summed
      pairVec_[i]->update(de/static_cast<double>(nPairs()));
    }
  }
}

void PairHybrid::writeRestart(const char* fileName) {
  writeRestartBase(fileName);
  std::ofstream file(fileName, std::ios_base::app);
  file << "# pairPrint " << pairPrint_ << endl;
  file << "# pairtypes " << nPairs() << endl;
  for (int i = 0; i < nPairs(); ++i) {
    stringstream ss;
    ss << fileName << i;
    file << "# pairRstFileName" << i << " " << ss.str() << endl;
    pairVec_[i]->writeRestart(ss.str().c_str());
  }

  if (selected_.size() > 0) {
    file << "# nselectedpairs " << selected_.size() << endl;
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      file << "# selectedpair" << i << "i " << selected_[i] << endl;
    }
  }
}

void PairHybrid::ipartNotPhysical(const int ipart) {
  if (selected_.size() > 0) {
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pairVec_[iPair]->ipartNotPhysical(ipart);
    }
  } else {
    for (int i = 0; i < nPairs(); ++i) {
      pairVec_[i]->ipartNotPhysical(ipart);
    }
  }
}

void PairHybrid::ipartIsPhysical(const int ipart) {
  if (selected_.size() > 0) {
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pairVec_[iPair]->ipartIsPhysical(ipart);
    }
  } else {
    for (int i = 0; i < nPairs(); ++i) {
      pairVec_[i]->ipartIsPhysical(ipart);
    }
  }
}

void PairHybrid::allPartPhysical() {
  if (selected_.size() > 0) {
    for (unsigned int i = 0; i < selected_.size(); ++i) {
      const int iPair = selected_[i];
      pairVec_[iPair]->allPartPhysical();
    }
  } else {
    for (int i = 0; i < nPairs(); ++i) {
      pairVec_[i]->allPartPhysical();
    }
  }
}

shared_ptr<PairHybrid> makePairHybrid(Space* space, const double rCut) {
  return make_shared<PairHybrid>(space, rCut);
}

#ifdef FEASST_NAMESPACE_
}  // namespace feasst
#endif  // FEASST_NAMESPACE_

