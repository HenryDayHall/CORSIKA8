/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <FLUKA.hpp>

namespace fluka {
  double (*rndmPtr)() = &rndm_interface;

  extern "C" {
  double flrndm_() { return ::fluka::rndmPtr(); }
  void flrnlp_(double* array, int const* N) {
    for (int i = 0; i < *N; ++i) { array[i] = ::fluka::rndmPtr(); }
  }

  //! overwrite function pointer to be used as FLUKA RNG (flrndm_())
  void setFlukaRNG(double (*func)()) { ::fluka::rndmPtr = func; }

  /**
   * The following (function) pointers make sure the corresponding objects in libflukahp.a
   * won't get dropped from the final file during linking.
   */

  [[maybe_unused]] extern auto* const hepevt_ptr = &hepevt_;
  [[maybe_unused]] extern auto* const stpxyc_ptr = &stpxyz_;
  [[maybe_unused]] extern auto* const evtxyz_ptr = &evtxyz_;
  [[maybe_unused]] extern auto* const sgmxyz_ptr = &sgmxyz_;
  [[maybe_unused]] extern auto* const ndmhep_ptr = &ndmhep_;
  }

} // namespace fluka
