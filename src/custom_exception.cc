/*
 * FEASST - Free Energy and Advanced Sampling Simulation Toolkit
 * http://pages.nist.gov/feasst, National Institute of Standards and Technology
 * Harold W. Hatch, harold.hatch@nist.gov
 *
 * Permission to use this data/software is contingent upon your acceptance of
 * the terms of LICENSE.txt and upon your providing
 * appropriate acknowledgments of NIST's creation of the data/software.
 */

#ifdef MPI_H_
  #include <mpi.h>
#endif
#ifdef _OPENMP
  #include <omp.h>
#endif  // _OPENMP
#include "./custom_exception.h"

#ifdef FEASST_NAMESPACE_
namespace feasst {
#endif  // FEASST_NAMESPACE_

CustomException::CustomException(std::string m
  ) {
  msg_ = m;
  catMessage();
  cout << msg_ << endl;
  std::cerr << msg_ << endl;
  exit(FAIL_CODE);  // terminate immediately for easy backtrace
}

CustomException::CustomException(std::stringstream& m) {
  msg_ = m.str();
  catMessage();
  cout << msg_ << endl;
  cout << "terminating" << endl;
  std::cerr << msg_ << endl;
  std::cerr << "terminating" << endl;
  #ifdef SEGERROR_
    // force seg fault
    int *foo = (int*)-1; // make a bad pointer
    printf("%d\n", *foo);       // causes segfault
  #else  // SEGERROR_
    exit(FAIL_CODE);
  #endif  // SEGERROR_
}

void CustomException::catMessage() {
  int nproc = 0;
  #ifdef MPI_H_
    int initialized;
    MPI_Initialized(&initialized);
    if (initialized) MPI_Comm_rank(MPI_COMM_WORLD, &nproc);
  #endif  // MPI_H_
  #ifdef _OPENMP
    nproc = omp_get_thread_num();
    #pragma omp critical
    {
  #endif  // _OPENMP

  std::stringstream message;
  message << "Throw on proc " << nproc << " : " << msg_;
  msg_.assign(message.str());

  #ifdef _OPENMP
    }
  #endif  // _OPENMP
}

#ifdef FEASST_NAMESPACE_
}  // namespace feasst
#endif  // FEASST_NAMESPACE_
