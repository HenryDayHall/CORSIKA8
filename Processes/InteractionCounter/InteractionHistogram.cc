/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/interaction_counter/InteractionHistogram.h>

#include <fstream>
#include <string>

using namespace corsika::process::interaction_counter;

void InteractionHistogram::fill(particles::Code projectile_id,
                                units::si::HEPEnergyType lab_energy,
                                units::si::HEPEnergyType mass_target, int A, int Z) {
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
        static_cast<corsika::particles::CodeIntType>(projectile_id), lab_energy / 1_GeV);
  }
}

void InteractionHistogram::saveLab(std::string const& filename) const {
  save(interaction_histogram_lab_, nuclear_inthist_lab_, filename, "lab system");
}

void InteractionHistogram::saveCMS(std::string const& filename) const {
  save(interaction_histogram_lab_, nuclear_inthist_cms_, filename,
       "center-of-mass system");
}

InteractionHistogram& InteractionHistogram::operator+=(
    InteractionHistogram const& other) {
  interaction_histogram_lab_ += other.interaction_histogram_lab_;
  interaction_histogram_cms_ += other.interaction_histogram_cms_;

  nuclear_inthist_lab_ += other.nuclear_inthist_lab_;
  nuclear_inthist_cms_ += other.nuclear_inthist_cms_;

  return *this;
}

InteractionHistogram InteractionHistogram::operator+(InteractionHistogram other) const {
  other.nuclear_inthist_lab_ += nuclear_inthist_lab_;
  other.nuclear_inthist_cms_ += nuclear_inthist_cms_;

  other.interaction_histogram_lab_ += interaction_histogram_lab_;
  other.interaction_histogram_cms_ += interaction_histogram_cms_;

  return other;
}

void InteractionHistogram::saveHist(hist_type const& hist, std::string const& filename,
                                    std::string const& comment) {
  std::ofstream file;
  file.open(filename);
  saveHist(hist, file, comment);
}

void InteractionHistogram::saveHist(hist_type const& hist, std::ofstream& file,
                                    std::string const& comment) {
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

void InteractionHistogram::saveHistMap(nuclear_hist_type const& histMap,
                                       std::ofstream& file) {
  file << "# nuclei" << std::endl;
  for (auto const& [pdg, hist] : histMap) {
    auto const num_ebins_nucl = hist.axis(0).size();

    file << pdg << ' ';
    for (int i = 0; i < num_ebins_nucl; ++i) { file << ' ' << hist.at(i); }
    file << std::endl;
  }
}

void InteractionHistogram::save(hist_type const& hist, nuclear_hist_type const& histMap,
                                std::string const& filename, std::string const& comment) {
  std::ofstream file;
  file.open(filename);

  saveHist(hist, file, comment);
  saveHistMap(histMap, file);
}

InteractionHistogram::InteractionHistogram()
    : interaction_histogram_cms_{boost::histogram::make_histogram(
          boost::histogram::axis::integer<short>(0, numIds),
          boost::histogram::axis::regular<double, boost::histogram::axis::transform::log>(
              num_bins_cms, lower_edge_cms, upper_edge_cms))}
    , interaction_histogram_lab_{boost::histogram::make_histogram(
          boost::histogram::axis::integer<short>(0, numIds),
          boost::histogram::axis::regular<double, boost::histogram::axis::transform::log>(
              num_bins_lab, lower_edge_lab, upper_edge_lab))} {}
