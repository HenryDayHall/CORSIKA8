/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_InteractionCounter_h
#define _corsika_InteractionCounter_h

#include <corsika/process/InteractionProcess.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

#include <boost/histogram.hpp>
#include <boost/histogram/ostream.hpp>
#include <fstream>
#include <functional>
#include <map>
#include <utility>

namespace corsika::process::interaction_counter {

  template <class TCountedProcess>
  class InteractionCounter
      : public InteractionProcess<InteractionCounter<TCountedProcess>> {
    static auto constexpr lower_edge_cms = 1., upper_edge_cms = 1e8;  // GeV sqrt s
    static auto constexpr lower_edge_lab = 1., upper_edge_lab = 1e12; // GeV lab
    static auto constexpr num_bins_lab = 120, num_bins_cms = 80;

    static auto constexpr numIds =
        static_cast<particles::CodeIntType>(particles::Code::LastParticle);

    using hist_type_count_cms = decltype(boost::histogram::make_histogram(
        boost::histogram::axis::integer<particles::CodeIntType>(0, numIds),
        boost::histogram::axis::regular<double, boost::histogram::axis::transform::log>(
            num_bins_cms, lower_edge_cms, upper_edge_cms)));

    using hist_type_count_lab = decltype(boost::histogram::make_histogram(
        boost::histogram::axis::integer<particles::CodeIntType>(0, numIds),
        boost::histogram::axis::regular<double, boost::histogram::axis::transform::log>(
            num_bins_lab, lower_edge_lab, upper_edge_lab)));

    static_assert(std::is_same_v<hist_type_count_cms, hist_type_count_lab>);

    using nucl_hist_type = decltype(boost::histogram::make_histogram(
        boost::histogram::axis::regular<double, boost::histogram::axis::transform::log>(
            num_bins_lab, lower_edge_lab, upper_edge_lab)));

    TCountedProcess& process_;
    hist_type_count_cms interaction_histogram_cms_;
    hist_type_count_lab interaction_histogram_lab_;

    std::map<int32_t, nucl_hist_type> nuclIntHistLab, nuclIntHistCMS;

  public:
    InteractionCounter(TCountedProcess& process)
        : process_(process)
        , interaction_histogram_cms_{boost::histogram::make_histogram(
              boost::histogram::axis::integer<short>(0, numIds),
              boost::histogram::axis::regular<double,
                                              boost::histogram::axis::transform::log>(
                  num_bins_cms, lower_edge_cms, upper_edge_cms))}
        , interaction_histogram_lab_{boost::histogram::make_histogram(
              boost::histogram::axis::integer<short>(0, numIds),
              boost::histogram::axis::regular<double,
                                              boost::histogram::axis::transform::log>(
                  num_bins_lab, lower_edge_lab, upper_edge_lab))} {}

    template <typename TProjectile>
    auto DoInteraction(TProjectile& projectile) {
      using namespace units::si;
      auto constexpr massAir = 15.994_GeV * (1.f - 0.7847) + 14.003_GeV * .7847;
      auto const sqrtS = sqrt(projectile.GetMass() * projectile.GetMass() +
                              massAir * massAir + 2 * projectile.GetEnergy() * massAir);

      if (projectile.GetPID() == corsika::particles::Code::Nucleus) {
        std::cerr << "NUCLEUS " << projectile.GetEnergy() << " "
                  << projectile.GetNuclearA() << " " << projectile.GetNuclearZ()
                  << std::endl;
        int const A = projectile.GetNuclearA();
        int const Z = projectile.GetNuclearZ();
        int32_t pdg = 1'000'000'000l + Z * 10'000l + A * 10l;

        if (nuclIntHistCMS.count(pdg) == 0) {

          nuclIntHistLab.emplace(
              pdg,
              boost::histogram::make_histogram(
                  boost::histogram::axis::regular<double,
                                                  boost::histogram::axis::transform::log>(
                      num_bins_lab, lower_edge_lab, upper_edge_lab)));

          nuclIntHistCMS.emplace(
              pdg,
              boost::histogram::make_histogram(
                  boost::histogram::axis::regular<double,
                                                  boost::histogram::axis::transform::log>(
                      num_bins_cms, lower_edge_cms, upper_edge_cms)));
        }

        nuclIntHistLab[pdg](projectile.GetEnergy() / 1_GeV);
        nuclIntHistCMS[pdg](sqrtS / 1_GeV);
      } else {
        interaction_histogram_cms_(
            static_cast<corsika::particles::CodeIntType>(projectile.GetPID()),
            sqrtS / 1_GeV);

        interaction_histogram_lab_(
            static_cast<corsika::particles::CodeIntType>(projectile.GetPID()),
            projectile.GetEnergy() / 1_GeV);
      }

      return process_.DoInteraction(projectile);
    }

    void Init() { process_.Init(); }

    template <typename TParticle>
    auto GetInteractionLength(TParticle const& particle) const {
      return process_.GetInteractionLength(particle);
    }

    auto CMSHists() const {
      return std::make_pair(&interaction_histogram_cms_, &nuclIntHistCMS);
    }

    auto labHists() const {
      return std::make_pair(&interaction_histogram_lab_, &nuclIntHistLab);
    }
  };

  template <typename T1, typename T2>
  static void saveHist(T1 const& hist, T2 const& histMap, std::string const& filename,
                       std::string const& comment = "") {
    auto const& energy_axis = hist.axis(1);
    std::ofstream myfile;
    myfile.open(filename);
    myfile << "# interaction count histogram (" << comment << ")" << std::endl
           << "# " << energy_axis.size() << " bins between " << energy_axis.bin(0).lower()
           << " and " << energy_axis.bin(energy_axis.size() - 1).upper() << " GeV"
           << std::endl;

    for (particles::CodeIntType p = 0;
         p < static_cast<particles::CodeIntType>(particles::Code::LastParticle); ++p) {

      if (auto pdg = static_cast<particles::PDGCodeType>(
              particles::GetPDG(static_cast<particles::Code>(p)));
          pdg < 1'000'000'000l) {
        myfile << "# " << static_cast<particles::Code>(p) << std::endl;
        myfile << pdg;
        for (int i = 0; i < energy_axis.size(); ++i) { myfile << ' ' << hist.at(p, i); }
        myfile << std::endl;
      }
    }

    myfile << "# nuclei" << std::endl;
    for (auto const& [pdg, hist] : histMap) {
      auto const num_ebins_nucl = hist.axis(0).size();
      assert(energy_axis.size() == num_ebins_nucl);

      myfile << pdg << " ";
      for (int i = 0; i < num_ebins_nucl; ++i) { myfile << ' ' << hist.at(i); }
      myfile << std::endl;
    }
  }
} // namespace corsika::process::interaction_counter
#endif
