#include "./trial_add.h"
#include "./mc.h"

#ifdef FEASST_NAMESPACE_
namespace feasst {
#endif  // FEASST_NAMESPACE_

TrialAdd::TrialAdd(const char* molType)
  : Trial(),
    molType_(molType) {
  defaultConstruction_();
}

TrialAdd::TrialAdd(Space *space,
  Pair *pair,
  Criteria *criteria,
  const char* molType)
  : Trial(space, pair, criteria),
    molType_(molType) {
  defaultConstruction_();
}

TrialAdd::TrialAdd(const char* fileName,
  Space *space,
  Pair *pair,
  Criteria *criteria)
  : Trial(space, pair, criteria, fileName) {
  defaultConstruction_();
  molType_ = fstos("molType", fileName);
}

void TrialAdd::writeRestart(const char* fileName) {
  writeRestartBase(fileName);
  std::ofstream file(fileName, std::ios_base::app);
  file << "# molType " << molType_ << endl;
}

void TrialAdd::defaultConstruction_() {
  className_.assign("TrialAdd");
  trialType_.assign("add");
  verbose_ = 0;
  molid_ = -1;
}

TrialAdd* TrialAdd::clone(Space* space, Pair *pair, Criteria *criteria) const {
  TrialAdd* t = new TrialAdd(*this);
  t->reconstruct(space, pair, criteria);
  return t;
}
shared_ptr<TrialAdd> TrialAdd::cloneShrPtr(
  Space* space, Pair* pair, Criteria* criteria) const {
  return(std::static_pointer_cast<TrialAdd, Trial>
    (cloneImpl(space, pair, criteria)));
}
shared_ptr<Trial> TrialAdd::cloneImpl(
  Space* space, Pair *pair, Criteria *criteria) const {
  shared_ptr<TrialAdd> t = make_shared<TrialAdd>(*this);
  t->reconstruct(space, pair, criteria);
  return t;
}


void TrialAdd::attempt1_() {
  WARN(verbose_ == 1, "attempting to " << trialType_);
  ASSERT((pair_->atomCut() != 1) || (space_->nMol() == space_->natom()) ||
         (!avbOn_), "this class assumes atomCut(" << pair_->atomCut()
         << ") == 0 when avb is on");

  // initialize molid_ if not already initialized from default value
  if (molid_ == -1) {
    molid_ = space_->findAddMolListIndex(molType_);
    ASSERT(molid_ != -1, "molType(" << molType_ << " not initialized.");
    std::stringstream ss;
    ss << "add" << molid_;
    trialType_.assign(ss.str());
  }

  if (confineFlag_ == 0) {
    // add molecule to entire box
    space_->addMol(molType_.c_str());

    // obtain vector of particle IDs of inserted molecule
    mpart_ = space_->lastMolIDVec();
  } else {
    // otherwise, add molecule only to confines
    bool inRegion = false;
    int tries = 0, maxTries = 1e4;
    while (!inRegion && (tries < maxTries)) {
      space_->addMol(molType_.c_str());
      mpart_ = space_->lastMolIDVec();
      if ( (space_->x(mpart_[0], confineDim_) <= confineUpper_) &&
           (space_->x(mpart_[0], confineDim_) >= confineLower_) ) {
        inRegion = true;
      } else {
        space_->delPart(mpart_);
      }
      ++tries;
    }
    ASSERT(tries != maxTries, "maximum number of attempts in confine");
  }
  pair_->addPart();

  // if particle types are constrained to be equiMolar
  if (space_->equiMolar() >= 1) {
    const int iMol = space_->mol().back(),
      iMolType = space_->molid()[iMol];
    if (space_->equiMolar() == 1) {
      const int nimt = space_->nMolType()[iMolType];

      // check if a different moltype has 2+ less in number
      for (int imt = 0; imt < space_->nMolTypes(); ++imt) {
        if (imt != iMolType) {
          if (nimt > space_->nMolType()[imt]+1) reject_ = 1;
        }
      }
    } else if (space_->equiMolar() == 2) {
      if (space_->nMol() % 2 == 0) {
        if (iMolType == 0) reject_ = 1;
      }
      if (space_->nMol() % 2 == 1) {
        if (iMolType == 1) reject_ = 1;
      }
    } else if (space_->equiMolar() == 3) {
      if (space_->nMol() % 2 == 0) {
        if (iMolType == 1) reject_ = 1;
      }
      if (space_->nMol() % 2 == 1) {
        if (iMolType == 0) reject_ = 1;
      }
    }
  }

  if (avbOn_ && (reject_ != 1)) {
    if (space_->nMol() > 1) {
      // HWH NOTE: Haven't implemented semigrand with AVB
      // select a random molecule that is different from mpart,
      // record its aggregation volume properties
      region_.assign("bonded");
      tmpart_ = space_->randMolDiff(space_->mol()[mpart_[0]]);
      const int nIn = static_cast<int>(pair_->neigh()
        [space_->mol()[tmpart_[0]]].size());
      preFac_ = vIn_*(space_->nMol()-1)/space_->nMol()/(nIn + 1);
      lnpMet_ = log(preFac_);

      // move mpart to bonded region of tmpart
      space_->avb(mpart_, tmpart_, rAbove_, rBelow_, region_.c_str());
      if (space_->cellType() > 0) {
        space_->updateCellofiMol(space_->mol()[mpart_.front()]);
      }
    }
  } else {
    const int iMolIndex = space_->findAddMolListIndex(molType_);
    const int nMolOfType = space_->nMolType()[iMolIndex];
    preFac_ = space_->vol()/static_cast<double>(nMolOfType);
    lnpMet_ = log(preFac_);
  }
  space_->wrap(mpart_);

  // multiple first bead insertion modifies def_, preFac_, tmpart_ for avb
  if ( (nf_ > 1) && (preFac_ != 0) && (reject_ != 1) ) {
    const double w = multiFirstBead_(3);
    lnpMet_ += log(w);
    preFac_ *= w;
    if (space_->cellType() > 0) {
      space_->updateCellofiMol(space_->mol()[mpart_.front()]);
    }
  }

  // record energy contribution of selected particle
  if ( (preFac_ != 0) && (reject_ != 1) ) {
    de_ = pair_->multiPartEner(mpart_, 3);
    pair_->update(mpart_, 3, "store");
    const int iMolIndex = space_->findAddMolListIndex(molType_);
    lnpMet_ += -criteria_->beta()*(de_ - def_)
            + log(criteria_->activ(iMolIndex));
    reject_ = 0;
  } else {
    reject_ = 1;
    de_ = 0;
    lnpMet_ = std::numeric_limits<double>::min();
  }

  if (criteria_->accept(lnpMet_, pair_->peTot() + de_, trialType_.c_str(),
                        reject_) == 1) {
    trialAccept_();
    pair_->update(mpart_, 3, "update");
    WARN(verbose_ == 1, "insertion accepted " << de_);

  // if not accepted, remove molecule, assuming a molecule is described
  // by sequentially listed particles
  } else {
    pair_->delPart(mpart_);
    space_->delPart(mpart_);
    WARN(verbose_ == 1, "insertion rejected " << de_);
    trialReject_();
  }
}

string TrialAdd::printStat(const bool header) {
  stringstream stat;
  stat << Trial::printStat(header);
  const int iMolIndex = space_->findAddMolListIndex(molType_);
  if (header) {
    stat << "N" << iMolIndex << " ";
  } else {
    const int nMolOfType = space_->nMolType()[iMolIndex];
    stat << nMolOfType << " ";
  }
  return stat.str();
}

void addTrial(MC *mc, const char* moltype) {
  shared_ptr<TrialAdd> trial = make_shared<TrialAdd>(moltype);
  mc->initTrial(trial);
}

void addTrial(shared_ptr<MC> mc, const char* moltype) {
  addTrial(mc.get(), moltype);
}

#ifdef FEASST_NAMESPACE_
}  // namespace feasst
#endif  // FEASST_NAMESPACE_
