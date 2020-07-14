/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <boost/histogram.hpp>
#include <boost/histogram/ostream.hpp>
#include <fstream>
#include <functional>
#include <map>
#include <utility>

namespace corsika::process::interaction_counter {
  using hist_type = decltype(boost::histogram::make_histogram(
      std::declval<boost::histogram::axis::integer<particles::CodeIntType>>(),
      std::declval<boost::histogram::axis::regular<
          double, boost::histogram::axis::transform::log>>()));

  using nucl_hist_type = decltype(boost::histogram::make_histogram(
      std::declval<boost::histogram::axis::regular<
          double, boost::histogram::axis::transform::log>>()));

  using nuclear_hist_type = std::map<int32_t, nucl_hist_type>;

  template <typename TKey, typename TVal>
  auto operator+=(std::map<TKey, TVal>& a, std::map<TKey, TVal> b) {
    a.merge(b);
    for (auto const& [id, hist] : b) { a[id] += hist; }
    return a;
  }

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
    InteractionHistogram();

    //! fill both CMS and lab histograms at the same time
    void fill(particles::Code projectile_id, units::si::HEPEnergyType lab_energy,
              units::si::HEPEnergyType mass_target, int A = 0, int Z = 0);

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

    void saveLab(std::string const& filename) const;

    void saveCMS(std::string const& filename) const;

    InteractionHistogram& operator+=(InteractionHistogram const& other);

    InteractionHistogram operator+(InteractionHistogram other) const;

  private:
    /*!
     * Save a histogram into a text file. The \arg comment string is written
     * into the header of the text file.
     * This method is static so that you can sum the histograms of multiple
     * InteractionProcesses before saving them into the same file.
     */

    static void saveHist(hist_type const& hist, std::string const& filename,
                         std::string const& comment = "");

    static void saveHist(hist_type const& hist, std::ofstream& file,
                         std::string const& comment = "");

    static void saveHistMap(nuclear_hist_type const& histMap, std::ofstream& file);

    /*!
     * save both the "normal" particle histograms as well as the "nuclear" histograms
     * into the same file
     */

    static void save(hist_type const& hist, nuclear_hist_type const& histMap,
                     std::string const& filename, std::string const& comment = "");
  };

} // namespace corsika::process::interaction_counter
