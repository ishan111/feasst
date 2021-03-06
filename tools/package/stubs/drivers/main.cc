#include "pair_lj.h"
#include "mc.h"
#include "trial_transform.h"

int main() {
  feasst::Space space(3, 0);
  for (int dim = 0; dim < space.dimen(); ++dim) {
    space.lset(8, dim); // 8 box length
  }
  space.addMolInit("../../forcefield/data.lj");
  feasst::PairLJ pair(&space, 3);   // potential truncation at 3
  pair.initEnergy();
  feasst::CriteriaMetropolis criteria(1.2, 1.);  // 1/kT = 1.2
  feasst::MC mc(&space, &pair, &criteria);
  feasst::transformTrial(&mc, "translate", 0.1);
  mc.nMolSeek(50, "../../forcefield/data.lj");  // add 50 particles
  mc.initLog("log", 1e4);
  mc.initMovie("movie", 1e4);
  mc.runNumTrials(1e6);
}
