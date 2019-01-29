
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/hadronic_elastic_model/HadronicElasticModel.h>

namespace corsika::process::HadronicElasticModel {

  void HadronicElasticInteraction::Init() {}

  HadronicElasticInteraction::HadronicElasticInteraction(
      corsika::environment::Environment const& env,
      corsika::units::si::CrossSectionType x, corsika::units::si::CrossSectionType y)
      : fX(x)
      , fY(y)
      , fEnvironment(env) {}

} // namespace corsika::process::HadronicElasticModel
