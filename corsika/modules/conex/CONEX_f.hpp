/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

//----------------------------------------------
//  C++ interface for the CONEX fortran code
//----------------------------------------------
// wrapper

#include <conexConfig.h>
#include <conexHEModels.h>
#include <array>

namespace conex {

  // the CORSIKA 8 random number interface

  /**
   * \function epos::rndm_interface
   *
   * this is the random number hook to external packages.
   *
   * CORSIKA8, for example, has to provide an implementation of this.
   **/
  extern float rndm_interface();

  /**
   * \function epos::double_rndm_interface
   *
   * this is the random number hook to external packages.
   *
   * CORSIKA8, for example, has to provide an implementation of this.
   **/

  extern double double_rndm_interface();

  extern "C" {}

  // the CONEX fortran interface

  extern "C" {
  extern struct { std::array<double, 16> dptl; } cxoptl_;

  void cegs4_(int&, int&);

  void initconex_(int&, int*, int&, int&,
#ifdef CONEX_EXTENSIONS
                  int&,
#endif
                  const char*, int);
  void conexrun_(int& ipart, double& energy, double& theta, double& phi,
                 double& injectionHeight, double& dimpact, int ioseed[3]);
  void conexcascade_();
  void hadroncascade_(int&, int&, int&, int&);
  void solvemomentequations_(int&);
  void show_(int& iqi, double& ei, double& xmi, double& ymi, double& zmi, double& dmi,
             double& xi, double& yi, double& zi, double& tmi, double& ui, double& vi,
             double& wi, int& iri, double& wti, int& latchi);

  int get_number_of_depth_bins_();

  void get_shower_data_(const int&, const int&, const int&, float&, float&, float&,
                        float&, float&);
  void get_shower_edep_(const int&, const int&, float&, float&);
  void get_shower_muon_(const int&, const int&, float&, float&);
  void get_shower_gamma_(const int&, const int&, float&);
  void get_shower_electron_(const int&, const int&, float&);
  void get_shower_hadron_(const int&, const int&, float&);
  }

} // namespace conex
