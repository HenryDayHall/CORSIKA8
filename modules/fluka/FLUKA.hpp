/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace fluka {
  size_t constexpr nmxhep = 10000;
  template <typename T>
  using hepmc_array = std::array<T, nmxhep>;

  extern "C" {
  /*
  c---------------------------------------------------------------------------
  c                   hep standard event commonblock.
  c---------------------------------------------------------------------------
  c---------------------------------------------------------------------------
  c
  c         nevhep      -   event number
  c         nhep        -   number of entries in the event record
  c
  c         isthep(i)   -   status code
  c         idhep(i)    -   particle id (particle data group standard)
  c
  c         jmohep(1,i) -   position of mother particle in list
  c         jmohep(2,i) -   position of second mother particle in list
  c         jdahep(1,i) -   position of first daughter in list
  c         jdahep(2,i) -   position of first daughter in list
  c
  c         phep(1,i)   -   p_x momentum in gev/c
  c         phep(2,i)   -   p_y momentum in gev/c
  c         phep(3,i)   -   p_z momentum in gev/c
  c         phep(4,i)   -   energy in gev
  c         phep(5,i)   -   mass in gev/c**2
  c
  c         vhep(1,i)   -   x position of production vertex in mm
  c         vhep(2,i)   -   y position of production vertex in mm
  c         vhep(3,i)   -   z position of production vertex in mm
  c         vhep(4,i)   -   time of production  in mm/c
  */

  extern struct {
    int nevhep;                  // event number
    int nhep;                    // number of entries
    hepmc_array<int> isthep;     // status code
    hepmc_array<int> idhep;      // PDG particle id
    hepmc_array<int[2]> jmohep;  // position of first, second mother
    hepmc_array<int[2]> jdahep;  // position of first, last daughter
    hepmc_array<double[5]> phep; // 4-momemtum, mass (GeV)
    hepmc_array<double[4]> vhep; // vertex, production time in mm
  } hepevt_;

  /*  Iflxyz must be consistent in all calls, to Stpxyz, Sgmxyz, Evtxyz
   *    Iflxyz =  1 -> only inelastic
   *    Iflxyz = 10 -> only elastic
   *    Iflxyz = 11 -> inelastic + elastic
   *    Iflxyz =100 -> only emd
   *    Iflxyz =101 -> inelastic + emd
   *    Iflxyz =110 -> elastic + emd
   *    Iflxyz =111 -> inelastic + elastic + emd
   */

  /*----------------------------------------------------------------------*
   *                                                                      *
   *     Copyright (C) 2022-2022      by    Alfredo Ferrari & Paola Sala  *
   *     All Rights Reserved.                                             *
   *                                                                      *
   *                                                                      *
   *     SeTuP for X/Y/Z interaction types:                               *
   *                                                                      *
   *     Authors:                           Alfredo Ferrari & Paola Sala  *
   *                                                                      *
   *                                                                      *
   *     Created on  24 October 2022  by    Alfredo Ferrari & Paola Sala  *
   *                                            Private        Private    *
   *                                                                      *
   *     Last change on  01-Nov-22    by             Alfredo Ferrari      *
   *                                                     Private          *
   *                                                                      *
   *     Input variables:                                                 *
   *                                                                      *
   *           Nmatfl    = Number of requested Fluka materials            *
   *           Nelmfl(m) = Number of elements in the m_th requested Fluka *
   *                       material, Nelmfl(m)=1 means simple element, no *
   *                       compound                                       *
   *           Izelfl(n) = Cumulative array of the Z of the elements con- *
   *                       stituting the requested Fluka materials        *
   *                      (the Z for the elements of the m_th materials   *
   *                       start at n = Sum_i=1^m-1 [Nelmfl(i)] + 1)      *
   *           Wfelml(n) = Cumulative array of the weight fractions of the*
   *                       elements constituting the requested Fluka      *
   *                       materials (the weight fractions for the elem-  *
   *                       ents of the m_th materials start at            *
   *                        n = Sum_i=1^m-1 [Nelmfl(i)] + 1)              *
   *           Mxelfl    = Dimension of the Iz/Wfelml arrays, it must be  *
   *                       Mxelfl >= Sum_i=1^nmatfl [Nelmfl(i)]           *
   *           Pptmax    = Maximum momentum (GeV/c) to be used in initia- *
   *                       lization (optional)                            *
   *           Ef2dp3    = Transition energy (GeV) from Fluka (Peanut) to *
   *                       Dpmjet3 for hA interactions (optional)         *
   *           Df2dp3    = Smearing (+/-Df2dp3) (GeV) of the transition   *
   *                       energy from Fluka (Peanut) to Dpmjet3 for hA   *
   *                       interactions (optional)                        *
   *           Lprint    = Material printout flag                         *
   *                                                                      *
   *     Output variables:                                                *
   *                                                                      *
   *           Mtflka(m) = Fluka material number corresponding to the m_th*
   *                       requested material                             *
   *                                                                      *
   *----------------------------------------------------------------------*/
  void stpxyz_(int const* NMATFL, int const NELMFL[], int const IZELFL[],
               double const WFELFL[], int const* MXELFL, double const* PPTMAX,
               double const* EF2DP3, double const* DF2DP3, int const* IFLXYZ,
               bool const* LPRINT, int* MTFLKA, char CRVRCK[8]);

  /*----------------------------------------------------------------------*
   *                                                                      *
   *     Copyright (C) 2022-2022      by    Alfredo Ferrari & Paola Sala  *
   *     All Rights Reserved.                                             *
   *                                                                      *
   *                                                                      *
   *     EVenT for X/Y/Z interaction types:                               *
   *                                                                      *
   *     Authors:                           Alfredo Ferrari & Paola Sala  *
   *                                                                      *
   *                                                                      *
   *     Created on  24 October 2022  by    Alfredo Ferrari & Paola Sala  *
   *                                            Private        Private    *
   *                                                                      *
   *     Last change on  28-Oct-22    by             Alfredo Ferrari      *
   *                                                     Private          *
   *                                                                      *
   *     Output variables:                                                *
   *                                                                      *
   *           Cumsgi(i) = cumulative macroscopic inelastic cross section *
   *                       (cm^2/g x rho) for the i_th element of the mmat*
   *                       material                                       *
   *           Cumsge(i) = cumulative macroscopic elastic   cross section *
   *                       (cm^2/g x rho) for the i_th element of the mmat*
   *                       material                                       *
   *           Cumsgm(i) = cumulative macroscopic emd       cross section *
   *                       (cm^2/g x rho) for the i_th element of the mmat*
   *                       material                                       *
   *                                                                      *
   *                                                                      *
   *----------------------------------------------------------------------*/
  void evtxyz_(int const* KPROJ, int const* MMAT, double const* EKIN, double const* PPROJ,
               double const* TXX, double const* TYY, double const* TZZ, int const* IFLXYZ,
               double CUMSGI[], double CUMSGE[], double CUMSGM[]);

  /*----------------------------------------------------------------------*
   *                                                                      *
   *     Copyright (C) 2022-2022      by    Alfredo Ferrari & Paola Sala  *
   *     All Rights Reserved.                                             *
   *                                                                      *
   *     SiGMa (mb) for X/Y/Z interaction types:                          *
   *                                                                      *
   *     Authors:                           Alfredo Ferrari & Paola Sala  *
   *                                                                      *
   *                                                                      *
   *     Created on  24 October 2022  by    Alfredo Ferrari & Paola Sala  *
   *                                            Private        Private    *
   *                                                                      *
   *     Last change on  27-Oct-22    by             Alfredo Ferrari      *
   *                                                     Private          *
   *                                                                      *
   *----------------------------------------------------------------------*/
  double sgmxyz_(int const* KPROJ, int const* MMAT, double const* EKIN,
                 double const* PPROJ, int const* IFLXYZ);

  /*----------------------------------------------------------------------*
   *                                                                      *
   *     Copyright (C) 2023-2023      by    Alfredo Ferrari & Paola Sala  *
   *     All Rights Reserved.                                             *
   *                                                                      *
   *     FiLL HEP common:                                                 *
   *                                                                      *
   *     Authors:                           Alfredo Ferrari & Paola Sala  *
   *                                                                      *
   *                                                                      *
   *     Created on 06 February 2023  by    Alfredo Ferrari & Paola Sala  *
   *                                            Private        Private    *
   *                                                                      *
   *     Last change on  07-Feb-23    by             Alfredo Ferrari      *
   *                                                     Private          *
   *                                                                      *
   *----------------------------------------------------------------------*/
  void fllhep_();
  }
} // namespace fluka
