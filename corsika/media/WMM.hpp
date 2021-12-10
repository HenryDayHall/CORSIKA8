/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <boost/filesystem.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/math/special_functions/legendre.hpp>

#include <cmath>
#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/utility/CorsikaData.hpp>
#include <fstream>

namespace corsika {
  /**
   * A magnetic field calculated with the WMM model
   *
   * @param  year        Year of the evaluation, between 2020 and 2025.
   * @param  altitude    Height of the location to evaluate the field at,
                         in km between -1 and 850.
   * @param  latitude    Latitude of the location to evaluate the field at,
                         in degrees between -90 and 90 (negative for southern hemisphere).
   * @param  longitute   Longitude of the location to evaluate the field at,
                         in degrees between -180 and 180 (negative for western
   hemisphere).
   *
   * @returns    The magnetic field vector in nT.
   *
   */

  inline MagneticFieldVector get_wmm(const CoordinateSystemPtr Cs, const double year,
                                     const LengthType altitude, const double latitude,
                                     const double longitude);
} // namespace corsika

#include <corsika/detail/media/WMM.inl>
