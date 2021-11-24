/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <string>

/**
 * @file qgsjet-II.04.hpp
 *
 * The interface to the fortran code.
 */

namespace qgsjetII {

  /**
   * This is the random number hook to external packages.
   *
   * CORSIKA8, for example, has to provide an implementation of this.
   */
  extern double rndm_interface();

} // namespace qgsjetII

//----------------------------------------------
//  C++ interface for the QGSJetII event generator
//----------------------------------------------
// wrapper

extern "C" {

// data memory layout

extern struct { int nsp; } qgarr12_;

const int nptmax = 95000;
const int iapmax = 208;

extern struct {
  double esp[nptmax][4];
  int ich[nptmax];
} qgarr14_;

extern struct {
  // c nsf - number of secondary fragments;
  // c iaf(i) - mass of the i-th fragment
  int nsf;
  int iaf[iapmax];
} qgarr13_;

extern struct {
  int nwt;
  int nwp;
} qgarr55_;

/**
 * Small helper class to provide a data-directory name in the format qgsjetII expects.
 */
class datadir {
private:
  datadir operator=(const std::string& dir);
  datadir operator=(const datadir&);

public:
  datadir(const std::string& dir);
  char data[132];
};

// functions
void qgset_();
void qgaini_(
    const char* datdir); // Note: there is a length limiation 132 from fortran-qgsjet here

/**
 * Additional initialization procedure per event.
 *
 * @param e0n  - interaction energy (per hadron/nucleon),
 * @param icp0 - hadron type (+-1 - pi+-, +-2 - p(p~), +-3 - n(n~), +-4 - K+-, +-5 -
 *               K_l/s),
 * @param iap  - projectile mass number (1 - for a hadron),
 * @param iat  - target mass number
 */
void qgini_(const double& e0n, const int& icp0, const int& iap, const int& iat);

/**
 * Generate one event configuration.
 */
void qgconf_();

/**
 * Hadron-nucleus (hadron-nucleus) particle production cross section.
 *
 * @param e0n lab. energy per projectile nucleon (hadron)
 * @param icz hadron class (1 - pion, 2 - nucleon, 3 - kaon)
 * @param iap0 projectile mass number (1=<iap0<=iapmax),
 * @param iat0 target mass number     (1=<iat0<=iapmax)
 */
double qgsect_(const double& e0n, const int& icz, const int& iap0, const int& iat0);

/**
 * Link to random number generation.
 */
double qgran_(int&);
}
