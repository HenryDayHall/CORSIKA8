
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_nuclib_interface_h_
#define _include_nuclib_interface_h_

extern "C" {

// nuclib common, NUClear Multiple Scattering
/*
    COMMON /CNUCMS/ B, BMAX, NTRY, NA, NB, NI, NAEL, NBEL
   +         ,JJA(IAMAX), JJB(IAMAX), JJINT(IAMAX,IAMAX)
   +         ,JJAEL(IAMAX), JJBEL(IAMAX)
 */

extern struct {
  double b, bmax;
  int ntry, na, nb, ni, nael, nbel;
  int jja[56], jjb[56], jjint[56][56], jjael[56], jjbel[56];
} cnucms_;

/*
  nuclib common, nuclear FRAGMENTS

  COMMON /FRAGMENTS/ PPP(3,60)
*/
extern struct { double ppp[60][3]; } fragments_;

// NUCLIB

// subroutine to initiate nuclib
void nuc_nuc_ini_();

// subroutine to sample nuclear interaction structure
void int_nuc_(const int&, const int&, const double&, const double&);

// subroutine to sample nuclear fragments
void fragm_(const int&, const int&, const int&, const double&, int&, int*);

void signuc_(const int&, const double&, double&);
}
#endif
