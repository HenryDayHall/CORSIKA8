/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <FLUKA.hpp>

#include <rng_impl.hpp>

IMPLEMENT_RNG(fluka)

namespace fluka {
  extern "C" {
  double flrndm_() { return ::draw_std_rnd(); }
  void flrnlp_(double* array, int const* N) { rng_ptr(array, *N); }

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
