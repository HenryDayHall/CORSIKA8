#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>

#include <fstream>
#include <string>

namespace corsika {

  /**
   * A magnetic field calculated with the WMM or IGRF model.
   */

  class WorldMagneticModel {

    /**
     * Internal data structure for a single shell of the spherical harmonic
     * parameterization.
     */
    struct ParameterLine {
      int n;
      int m;
      double g;
      double h;
      double dg; //< time dependence of g in "per year"
      double dh; //< time dependence of h in "per year"
    };

  public:
    /**
     * Construct a new World Magnetic Model object.
     *
     * @param center Center of Earth.
     * @param data Data table to read.
     */
    WorldMagneticModel(Point const& center, std::string const& data = "GeoMag/WMM.COF");

    /**
     * Calculates the value of the magnetic field.
     *
     * @param  year        Year of the evaluation, between 2020 and 2025.
     * @param  altitude    Height of the location to evaluate the field at,
     *                    in km between -1 and 850.
     * @param  latitude    Latitude of the location to evaluate the field at,
     *                   in degrees between -90 and 90 (negative for southern hemisphere).
     * @param  longitute   Longitude of the location to evaluate the field at,
     *                      in degrees between -180 and 180 (negative for western
     *   hemisphere).
     *
     * @returns    The magnetic field vector in nT.
     */
    MagneticFieldVector getField(double const year, LengthType const altitude,
                                 double const latitude, double const longitude);

  private:
    Point center_;                                         //< Center of Earth.
    std::map<int, std::vector<ParameterLine>> parameters_; //< epoch to Parameters map
  };
} // namespace corsika

#include <corsika/detail/media/WorldMagneticModel.inl>