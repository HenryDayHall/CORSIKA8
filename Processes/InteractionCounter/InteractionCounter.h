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

template <typename TKey, typename TVal>
auto operator+=(std::map<TKey, TVal>& a, std::map<TKey, TVal> b) {
  a.merge(b);
  for (auto const& [id, hist] : b) { a[id] += hist; }
  return a;
}

namespace corsika::process::interaction_counter {
  using hist_type = decltype(boost::histogram::make_histogram(
      std::declval<boost::histogram::axis::integer<particles::CodeIntType>>(),
      std::declval<boost::histogram::axis::regular<
          double, boost::histogram::axis::transform::log>>()));

  using nucl_hist_type = decltype(boost::histogram::make_histogram(
      std::declval<boost::histogram::axis::regular<
          double, boost::histogram::axis::transform::log>>()));

  using nuclear_hist_type = std::map<int32_t, nucl_hist_type>;

  class InteractionHistogram {
    static auto constexpr lower_edge_cms = 1., upper_edge_cms = 1e8;  // GeV sqrt s
    static auto constexpr lower_edge_lab = 1., upper_edge_lab = 1e12; // GeV lab
    static auto constexpr num_bins_lab = 120, num_bins_cms = 80;
    static auto constexpr numIds =
        static_cast<particles::CodeIntType>(particles::Code::LastParticle);

    hist_type interaction_histogram_cms_;
    hist_type interaction_histogram_lab_;

    /*!
     * These maps map PDG nuclei codes to their corresponding interaction histograms
     */
    nuclear_hist_type nuclear_inthist_lab_, nuclear_inthist_cms_;

  public:
    InteractionHistogram()
        : interaction_histogram_cms_{boost::histogram::make_histogram(
              boost::histogram::axis::integer<short>(0, numIds),
              boost::histogram::axis::regular<double,
                                              boost::histogram::axis::transform::log>(
                  num_bins_cms, lower_edge_cms, upper_edge_cms))}
        , interaction_histogram_lab_{boost::histogram::make_histogram(
              boost::histogram::axis::integer<short>(0, numIds),
              boost::histogram::axis::regular<double,
                                              boost::histogram::axis::transform::log>(
                  num_bins_lab, lower_edge_lab, upper_edge_lab))} {}

    void fill(particles::Code projectile_id, units::si::HEPEnergyType lab_energy,
              units::si::HEPEnergyType mass_target, int A = 0, int Z = 0) {
      using namespace units::si;

      if (projectile_id == particles::Code::Nucleus) {
        auto const sqrtS =
            sqrt(A * A * (units::constants::nucleonMass * units::constants::nucleonMass) +
                 mass_target * mass_target + 2 * lab_energy * mass_target);

        int32_t const pdg = 1'000'000'000l + Z * 10'000l + A * 10l;

        if (nuclear_inthist_cms_.count(pdg) == 0) {
          nuclear_inthist_lab_.emplace(
              pdg,
              boost::histogram::make_histogram(
                  boost::histogram::axis::regular<double,
                                                  boost::histogram::axis::transform::log>(
                      num_bins_lab, lower_edge_lab, upper_edge_lab)));

          nuclear_inthist_cms_.emplace(
              pdg,
              boost::histogram::make_histogram(
                  boost::histogram::axis::regular<double,
                                                  boost::histogram::axis::transform::log>(
                      num_bins_cms, lower_edge_cms, upper_edge_cms)));
        }

        nuclear_inthist_lab_[pdg](lab_energy / 1_GeV);
        nuclear_inthist_cms_[pdg](sqrtS / 1_GeV);
      } else {
        auto const projectile_mass = particles::GetMass(projectile_id);
        auto const sqrtS = sqrt(projectile_mass * projectile_mass +
                                mass_target * mass_target + 2 * lab_energy * mass_target);

        interaction_histogram_cms_(
            static_cast<corsika::particles::CodeIntType>(projectile_id), sqrtS / 1_GeV);

        interaction_histogram_lab_(
            static_cast<corsika::particles::CodeIntType>(projectile_id),
            lab_energy / 1_GeV);
      }
    }

    auto CMSHists() const {
      return std::pair<decltype(interaction_histogram_cms_) const&,
                       decltype(nuclear_inthist_cms_) const&>{interaction_histogram_cms_,
                                                              nuclear_inthist_cms_};
    }

    auto labHists() const {
      return std::pair<decltype(interaction_histogram_lab_) const&,
                       decltype(nuclear_inthist_lab_) const&>(interaction_histogram_lab_,
                                                              nuclear_inthist_lab_);
    }

    void saveLab(std::string const& filename) const {
      save(interaction_histogram_lab_, nuclear_inthist_lab_, filename, "lab system");
    }

    void saveCMS(std::string const& filename) const {
      save(interaction_histogram_lab_, nuclear_inthist_cms_, filename,
           "center-of-mass system");
    }

    auto& operator+=(InteractionHistogram const& other) {
      interaction_histogram_lab_ += other.interaction_histogram_lab_;
      interaction_histogram_cms_ += other.interaction_histogram_cms_;

      nuclear_inthist_lab_ += other.nuclear_inthist_lab_;
      nuclear_inthist_cms_ += other.nuclear_inthist_cms_;

      return *this;
    }

    auto operator+(InteractionHistogram other) const {
      other.nuclear_inthist_lab_ += nuclear_inthist_lab_;
      other.nuclear_inthist_cms_ += nuclear_inthist_cms_;

      return other;
    }

  private:
    /*! Save a histogram into a text file. The \arg comment string is written
     * into the header of the text file.
     * This method is static so that you can sum the histograms of multiple
     * InteractionProcesses before saving them into the same file.
     */

    static void saveHist(hist_type const& hist, std::string const& filename,
                         std::string const& comment = "") {
      std::ofstream file;
      file.open(filename);
      saveHist(hist, file, comment);
    }

    static void saveHist(hist_type const& hist, std::ofstream& file,
                         std::string const& comment = "") {
      auto const& energy_axis = hist.axis(1);

      file << "# interaction count histogram (" << comment << ")" << std::endl
           << "# " << energy_axis.size() << " bins between " << energy_axis.bin(0).lower()
           << " and " << energy_axis.bin(energy_axis.size() - 1).upper() << " GeV"
           << std::endl;

      for (particles::CodeIntType p = 0;
           p < static_cast<particles::CodeIntType>(particles::Code::LastParticle); ++p) {

        if (auto pdg = static_cast<particles::PDGCodeType>(
                particles::GetPDG(static_cast<particles::Code>(p)));
            pdg < 1'000'000'000l) {
          file << "# " << static_cast<particles::Code>(p) << std::endl;
          file << pdg;
          for (int i = 0; i < energy_axis.size(); ++i) { file << ' ' << hist.at(p, i); }
          file << std::endl;
        }
      }
    }

    static void saveHistMap(nuclear_hist_type const& histMap, std::ofstream& file) {
      file << "# nuclei" << std::endl;
      for (auto const& [pdg, hist] : histMap) {
        auto const num_ebins_nucl = hist.axis(0).size();

        file << pdg << ' ';
        for (int i = 0; i < num_ebins_nucl; ++i) { file << ' ' << hist.at(i); }
        file << std::endl;
      }
    }

    /*!
     * save both the "normal" particle histograms as well as the "nuclear" histograms
     * into the same file
     */

    static void save(hist_type const& hist, nuclear_hist_type const& histMap,
                     std::string const& filename, std::string const& comment = "") {
      std::ofstream file;
      file.open(filename);

      saveHist(hist, file, comment);
      saveHistMap(histMap, file);
    }
  };

  /*!
   * Wrapper around an InteractionProcess that fills histograms of the number
   * of calls to DoInteraction() binned in projectile energy (both in
   * lab and center-of-mass frame) and species
   */
  template <class TCountedProcess>
  class InteractionCounter
      : public InteractionProcess<InteractionCounter<TCountedProcess>> {

    TCountedProcess& process_;
    InteractionHistogram histogram_;

  public:
    InteractionCounter(TCountedProcess& process)
        : process_(process) {}

    template <typename TProjectile>
    auto DoInteraction(TProjectile& projectile) {
      auto const massNumber = projectile.GetNode()
                                  ->GetModelProperties()
                                  .GetNuclearComposition()
                                  .GetAverageMassNumber();
      auto const massTarget = massNumber * units::constants::nucleonMass;

      if (auto const projectile_id = projectile.GetPID();
          projectile_id == particles::Code::Nucleus) {
        auto const A = projectile.GetNuclearA();
        auto const Z = projectile.GetNuclearZ();
        histogram_.fill(projectile_id, projectile.GetEnergy(), massTarget, A, Z);
      } else {
        histogram_.fill(projectile_id, projectile.GetEnergy(), massTarget);
      }
      return process_.DoInteraction(projectile);
    }

    void Init() { process_.Init(); }

    template <typename TParticle>
    auto GetInteractionLength(TParticle const& particle) const {
      return process_.GetInteractionLength(particle);
    }

    auto const& GetHistogram() const { return histogram_; }
  };

} // namespace corsika::process::interaction_counter

#endif
