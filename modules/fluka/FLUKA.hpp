/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace fluka {
extern "C" {
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
*----------------------------------------------------------------------*
*/
void stpxyz_(int const* NMATFL,
    int const NELMFL[],
    int const IZELFL[],
    double const WFELFL[],
    int const* MXELFL,
    double const* PPTMAX,
    double const* EF2DP3,
    double const* DF2DP3,
    int const* IFLXYZ,
    bool const* LPRINT,
    int* MTFLKA,
    char CRVRCK[8]);
}
}
