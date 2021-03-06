"""
 * FEASST - Free Energy and Advanced Sampling Simulation Toolkit
 * http://pages.nist.gov/feasst, National Institute of Standards and Technology
 * Harold W. Hatch, harold.hatch@nist.gov
 *
 * Permission to use this data/software is contingent upon your acceptance of
 * the terms of LICENSE.txt and upon your providing
 * appropriate acknowledgments of NIST's creation of the data/software.
"""

# # The following three lines are an alternative method to link _feasst.so
# import os, sys
# feasstdir = os.getenv("FEASST_INSTALL_DIR_") + "/build"
# sys.path.append(feasstdir)

import feasst
feasst.ranInitByDate()
space = feasst.makeSpace(3)
space.initBoxLength(8)
space.addMolInit("../forcefield/data.lj")
pair = feasst.PairLJ(space, 3)    # potential truncation at 3
pair.initEnergy()
criteria = feasst.CriteriaMetropolis(1.2, 1.);  # 1/kT = 1.2
mc = feasst.MC(space, pair, criteria)
maxMoveParam = 0.1
feasst.transformTrial(mc, "translate", maxMoveParam)
mc.nMolSeek(50)   # add 50 particles
mc.initLog("log", int(1e4))
mc.initMovie("movie", int(1e4))
mc.runNumTrials(int(1e6))
# mc.initWindows(1)       # initialize parallel windows
# mc.runNumSweeps(1, -1)  # run until all windows sweep once

