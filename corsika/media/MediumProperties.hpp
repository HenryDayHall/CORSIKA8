/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  /**
   * Medium types are useful most importantly for effective models
   * like energy losses. a particular medium (mixture of components)
   * may have specif properties not reflected by its mixture of
   * components.
   **/

  enum class MediumType {
    Unknown,
    Element,
    RadioactiveElement,
    InorganicCompound,
    OrganicCompound,
    Polymer,
    Mixture,
    BiologicalDosimetry
  };

  enum class State { Unknown, Solid, Liquid, Gas, DiatomicGas };

  enum class Medium : int16_t;
  using MediumIntType = std::underlying_type<Medium>::type;

  /** \todo documentation needs update ...
   * \struct MediumData
   *
   * Simple object to group together a number of properties
   *
   **/
  struct MediumData {
    std::string name_;
    std::string pretty_name_;
    double weight_;
    int weight_significant_figure_;
    int weight_error_last_digit_;
    double Z_over_A_;
    double sternheimer_density_;
    double corrected_density_;
    State state_;
    MediumType type_;
    std::string symbol_;
    double Ieff_;
    double Cbar_;
    double x0_;
    double x1_;
    double aa_;
    double sk_;
    double dlt0_;

    std::string getName() const { return name_; }
    std::string getPrettyName() const { return pretty_name_; }
    double getWeight() const { return weight_; }
    const int& weight_significant_figure() const { return weight_significant_figure_; }
    const int& weight_error_last_digit() const { return weight_error_last_digit_; }
    const double& Z_over_A() const { return Z_over_A_; }
    double getSternheimerDensity() const { return sternheimer_density_; }
    double getCorrectedDensity() const { return corrected_density_; }
    State getState() const { return state_; }
    MediumType getType() const { return type_; }
    std::string getSymbol() const { return symbol_; }
    double getIeff() const { return Ieff_; }
    double getCbar() const { return Cbar_; }
    double getX0() const { return x0_; }
    double getX1() const { return x1_; }
    double getAA() const { return aa_; }
    double getSK() const { return sk_; }
    double getDlt0() const { return dlt0_; }
  };

} // namespace corsika

#include <corsika/media/GeneratedMediaProperties.inc>

namespace corsika {

  constexpr MediumData const& mediumData(Medium const m) {
    return corsika::detail::medium_data[static_cast<MediumIntType>(m)];
  }

} // namespace corsika
