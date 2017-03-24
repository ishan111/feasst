#include <gtest/gtest.h>
#include "pair_squarewell.h"

TEST(PairSquareWell, mickeymouse) {
  const double rCut = 1.02;
  Space s(3, 0);
  for (int dim=0; dim < s.dimen(); ++dim) s.lset(20,dim);
  s.addMolInit("../forcefield/data.cg3_91_0.57_2");
  vector<double> xAdd(s.dimen());
  s.xAdd = xAdd;
  s.addMol("../forcefield/data.cg3_91_0.57_2");
  xAdd[1] = 2*0.266345520433943000 + 1.01;
  s.xAdd = xAdd;
  s.addMol("../forcefield/data.cg3_91_0.57_2");
  // flip
  s.qMolAlt(1, 0, 1);
  s.qMolAlt(1, 3, 0);
  s.quat2pos(1);
  PairSquareWell p(&s, rCut);
  p.initLMPData("../forcefield/data.cg3_91_0.57_2");
  p.rCutijset(1, 1, rCut);
  p.initHardSphere(1, 2);
  p.initHardSphere(2, 2);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), -1, doubleTolerance);
  EXPECT_EQ(p.checkEnergy(doubleTolerance, 0), 1);

  // write restart, read restart and test
  { p.writeRestart("tmp/p");
    PairSquareWell p2(&s, "tmp/p");
    p2.initEnergy();
    EXPECT_NEAR(p2.peTot(), -1, doubleTolerance);
  }

  xAdd[1] = 0.01000001;
  s.transMol(1, xAdd);
  p.initEnergy();
  EXPECT_NEAR(p.peTot(), 0, doubleTolerance);
  EXPECT_EQ(p.checkEnergy(doubleTolerance, 1), 1);
  xAdd[1] = -2*0.01000001;
  s.transMol(1, xAdd);
  p.initEnergy();
  EXPECT_GE(p.peTot(), 1e100);

  // flip particle again
  s.qMolAlt(1, 0, 0);
  s.qMolAlt(1, 3, 1);
  s.quat2pos(1);
  xAdd[1] = -s.x(4, 1) + 0.83 + 1*0.3966345520433943000;
  s.transMol(1, xAdd);
  p.initEnergy();
  EXPECT_GE(p.peTot(), 1e100);
  xAdd[1] = 0.02;
  s.transMol(1, xAdd);
  p.initEnergy();
  EXPECT_EQ(p.peTot(), 0);
  EXPECT_EQ(p.checkEnergy(doubleTolerance, 1), 1);

  // write restart, read restart and test
  { p.writeRestart("tmp/p");
    PairSquareWell p2(&s, "tmp/p");
    p2.initEnergy();
    EXPECT_EQ(p2.peTot(), 0);
  }

  // test again
  xAdd[0] = s.x(2,0);
  xAdd[1] = -s.x(4,1)-0.133172760216972000-0.5*1.85-s.x(1,1)+1e-10;
  s.transMol(1, xAdd);
  p.initEnergy();
  EXPECT_GE(p.peTot(), 1e100);
  xAdd[0] = 0.;
  xAdd[1] = -2e-10;
  s.transMol(1, xAdd);
  p.initEnergy();
  EXPECT_EQ(p.peTot(), 0);
}
