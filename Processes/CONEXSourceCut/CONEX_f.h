/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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

namespace conex {

  extern "C" {
  // ipart,energy,theta,phi,dimpact,ioseed
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
