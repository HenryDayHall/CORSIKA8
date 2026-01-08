/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/interfaces/IRefractiveIndexModel.hpp>

#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

namespace corsika {
  namespace media {

    template <typename T>
    template <typename... Args>
    LinearTabulatedRefractiveIndex<T>::LinearTabulatedRefractiveIndex(
        std::string const& tabulated_amosphere_path, LengthType const radius,
        Args&&... args)
        : T(std::forward<Args>(args)...)
        , tabulated_amosphere_(tabulated_amosphere_path)
        , local_cs(make_translation(get_root_CoordinateSystem(), {0_m, 0_m, radius}))
        , radius_(radius) {

      // Interpolate between the tabulated points in higher precision
    }

    template <typename T>
    std::vector<RefractiveIndexData> LinearTabulatedRefractiveIndex<T>::parse_file(
        std::string_view const& tabulated_amosphere_path) {
      // CSV read from file
      std::ifstream file(tabulated_amosphere_);
      std::string line;
      int i = 0;

      std::vector<RefractiveIndexData> refractive_index_data;

      while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string height;
        std::string refractive_index;
        std::getline(iss, height, ',');
        std::getline(iss, refractive_index, ',');
        // remove whitespaces
        height.erase(std::remove_if(height.begin(), height.end(), ::isspace),
                     height.end());
        refractive_index.erase(
            std::remove_if(refractive_index.begin(), refractive_index.end(), ::isspace),
            refractive_index.end());

        if (height.empty() || refractive_index.empty()) { continue; }

        if (height.find_first_not_of("0123456789.") != std::string::npos) { continue; }

        refractive_index_data.push_back({std::stod(height), std::stod(refractive_index)});
        i++;
      }

      std::sort(refractive_index_data.begin(), refractive_index_data.end(),
                [](RefractiveIndexData const& a, RefractiveIndexData const& b) {
                  return a.height < b.height;
                });

      return refractive_index_data;
    }

    void LinearTabulatedRefractiveIndex<T>::interpolate(
        std::vector<RefractiveIndexData> const& refractive_index_data) {

      if (refractive_index_data[0].first < refractive_index_data[1].first) {
        std::cerr << "Error in tabulated atmosphere file: heights are not sorted in "
                     "ascending order"
                  << std::endl;
        throw(std::runtime_error(
            "Error in tabulated atmosphere file: heights are not sorted "
            "in ascending order"));
      }

      for (int j = 1; j < height.size(); j++) {

        double data_height1 = refractive_index_data[j - 1].height;
        double data_height2 = refractive_index_data[j].height;

        double data_refractive_index1 = std::log(refractive_index_data[j - 1].second);
        double data_refractive_index2 = std::log(refractive_index_data[j].second);

        for (int interpol_height = 0; i < refractive_index_profile_.size(); i++) {

          if (interpol_height >= data_height2) { break; }

          double height_diff = data_height2 - data_height1;
          double refractive_index_diff = data_refractive_index2 - data_refractive_index1;

          double fraction =
              (static_cast<double>(interpol_height) - data_height1) / height_diff;

          refractive_index_profile_[interpol_height] =
              std::exp(data_refractive_index1 + fraction * refractive_index_diff);
        }
      }
    }

    template <typename T>
    inline double LinearTabulatedRefractiveIndex<T>::getRefractiveIndex(
        Point const& point) const {
      double distance = distance(point, center_).magnitude() - radius_;
      if (distance < 0) {
        throw(std::runtime_error(
            "LinearTabulatedRefractiveIndex: point is inside the earth"));
      }
      if (distance > 120'000) { return 1.0; }

      int lower = std::floor(distance / 1.0);
      int upper = std::ceil(distance / 1.0);
      double lower_index = refractive_index_profile_[lower];
      double upper_index = refractive_index_profile_[upper];
      double fraction = distance - lower;
      return lower_index + fraction * (upper_index - lower_index);
    }

  } // namespace media
} // namespace corsika