/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/interfaces/IRefractiveIndexModel.hpp>

namespace corsika {
  namespace media {
    /**
     * An exponential refractive index.
     *
     * This class returns the value of an exponential refractive index
     * for all evaluated locations.
     *
     */
    template <typename T, typename geometry_transform>
    class LinearTabulatedRefractiveIndex : public T {

      std::string tabulated_amosphere_;

      struct RefractiveIndexData {
        double height;
        double refractive_index;
      };

      std::array<double, 120000> refractive_index_profile_;

      CoordinateSystemPtr local_cs;

    protected:
      std::vector<RefractiveIndexData> parse_file(
          std::string_view const& tabulated_amosphere_path);
      void interpolate(std::vector<RefractiveIndexData> const& refractive_index_data);

    public:
      /**
       * Construct an LinearTabulatedRefractiveIndex.
       *
       * This is initialized with the path to a tabulated atmosphere in csv format with
       * two columns (height, refractive index) and returns the value of the refractive
       * index at a given point location.
       *
       * @param field    The refractive index to return to a given point.
       */
      template <typename... Args>
      LinearTabulatedRefractiveIndex(std::string const& tabulated_amosphere_path,
                                     LengthType const radius, Args&&... args);

      /**
       * Evaluate the refractive index at a given location using its z-coordinate.
       *
       * @param  point    The location to evaluate at.
       * @returns    The refractive index at this point.
       */
      double getRefractiveIndex(Point const& point) const final override;

    }; // END: class LinearTabulatedRefractiveIndex

  } // namespace media
} // namespace corsika

#include <corsika/detail/media/refractivity/LinearTabulatedRefractiveIndex.inl>