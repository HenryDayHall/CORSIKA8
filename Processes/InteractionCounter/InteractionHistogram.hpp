/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/SaveBoostHistogram.hpp>

#include <boost/histogram.hpp>
#include <fstream>
#include <functional>
#include <map>
#include <utility>

namespace corsika::process::interaction_counter {
  namespace detail {
    inline auto hist_factory(unsigned int bin_number, float e_low, float e_high) {
      namespace bh = boost::histogram;
      namespace bha = bh::axis;

      auto h = bh::make_histogram(
          bha::category<int, bh::use_default, bha::option::growth_t>{{2212, 2112},
                                                                     "projectile PDG"},
          bha::regular<float, bha::transform::log>{bin_number, e_low, e_high,
                                                   "energy/eV"});
      return h;
    }
  } // namespace detail

  class InteractionHistogram {
    static double constexpr lower_edge_cms = 1e3, upper_edge_cms = 1e17; // eV sqrt s
    static double constexpr lower_edge_lab = 1e3, upper_edge_lab = 1e21; // eV lab
    static unsigned int constexpr num_bins_lab = 18 * 10, num_bins_cms = 14 * 10;

    using hist_type =
        decltype(detail::hist_factory(num_bins_lab, lower_edge_lab, upper_edge_lab));

    hist_type inthist_cms_, inthist_lab_;

  public:
    InteractionHistogram();

    //! fill both CMS and lab histograms at the same time
    void fill(particles::Code projectile_id, units::si::HEPEnergyType lab_energy,
              units::si::HEPEnergyType mass_target, int A = 0, int Z = 0);

    hist_type const& CMSHist() const { return inthist_cms_; }

    hist_type const& labHist() const { return inthist_lab_; }

    void saveLab(std::string const& filename) const;

    void saveCMS(std::string const& filename) const;

    InteractionHistogram& operator+=(InteractionHistogram const& other);

    InteractionHistogram operator+(InteractionHistogram other) const;
  };

} // namespace corsika::process::interaction_counter
