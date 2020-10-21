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

#include <string>

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
   Small helper class to provide a data-directory name in the format qgsjetII expects
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
   @function qgini_

   additional initialization procedure per event

   @parameter e0n  - interaction energy (per hadron/nucleon),
   @parameter icp0 - hadron type (+-1 - pi+-, +-2 - p(p~), +-3 - n(n~), +-4 - K+-, +-5 -
   K_l/s),
   @parameter iap  - projectile mass number (1 - for a hadron),
   @parameter iat  - target mass number
*/
void qgini_(const double& e0n, const int& icp0, const int& iap, const int& iat);

/**
   @function qgconf_

   generate one event configuration
*/
void qgconf_();

/**
   @function qgsect_

   hadron-nucleus (hadron-nucleus) particle production cross section

   @parameter e0n lab. energy per projectile nucleon (hadron)
   @parameter icz hadron class (1 - pion, 2 - nucleon, 3 - kaon)
   @parameter iap projectile mass number (1=<iap<=iapmax),
   @parameter iat target mass number     (1=<iat<=iapmax)
 */
double qgsect_(const double& e0n, const int& icz, const int& iap0, const int& iat0);

/**
   @function qgran

   link to random number generation
 */
double qgran_(int&);

/**
   dummy function from CRMC
 */
void lzmaopenfile_(const char* name, int length);
void lzmaclosefile_();
void lzmafillarray_(const double& dum, const int& idum);
}

#include <corsika/detail/modules/qgsjetII/qgsjet-II-04.inl>

/**
 * WARNING, TODO, FIXME: this below here has to go away, this is just a dummy until
 * we "properly" link to the external corsika-data submodule
 */

namespace corsika_data {

  void CorDataOpenFile(const std::string&) {
    throw std::runtime_error(
        "CorDataOpenFile: Cannot read compressed data files with dummy library.");
  }
  void CorDataFillArray(double*, const int&) {
    throw std::runtime_error(
        "CorDataFillArray: Cannot read compressed data files with dummy library.");
  }
  void CorDataCloseFile() {
    throw std::runtime_error(
        "CorDataCloseFile: Cannot read compressed data files with dummy library.");
  }
  double CorDataNextNumber() {
    throw std::runtime_error(
        "CorDataNextNumber: Cannot read compressed data files with dummy library.");
    return 0;
  }
  void CorDataNextText(std::string&) {
    throw std::runtime_error(
        "CorDataNextText(string&): Cannot read compressed data files with dummy "
        "library.");
  }
  void CorDataNextText(char*, int) {
    throw std::runtime_error(
        "CorDataNextText(char*): Cannot read compressed data files with dummy library.");
  }
  bool CorDataCanDeCompress() { return false; }

  // the fortran interface functions
  extern "C" {
  void cordataopenfile_(const char* name) { CorDataOpenFile(name); }
  void cordatafillarray_(double* data, const int& length) {
    CorDataFillArray(data, length);
  }
  void cordataclosefile_() { CorDataCloseFile(); }
  double cordatanextnumber_() { return CorDataNextNumber(); }
  void cordatanexttext_(char*, int) {
    throw std::runtime_error(
        "cordatanexttext_: Cannot read compressed data files with dummy library.");
  }
  int cordatacandecompress_() { return 0; }
  }
} // namespace corsika_data
