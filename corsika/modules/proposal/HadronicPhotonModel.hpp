/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>

namespace corsika::proposal {

  //! Implements the production of secondary hadrons for the hadronic interaction of real
  //! and virtual photons. At high energies an external model
  //! is needed that implements the doInteraction(TSecondaries& view, Code const
  //! projectile, Code const target,FourMomentum const& projectileP4, FourMomentum const&
  //! targetP4) routine. Low energy interactions are currently not implemented. The
  //! threshold between LE and HE interactions is defined in lab energy.
  //! @tparam THadronicModel

  template <class THadronicModel>
  class HadronicPhotonModel {
  public:
    HadronicPhotonModel(THadronicModel&, HEPEnergyType const&);
    //!
    //! Calculate produce the hadronic secondaries in a hadronic photon interaction and
    //! store them on the particle stack.
    //!
    template <typename TSecondaryView>
    ProcessReturn doHadronicPhotonInteraction(TSecondaryView&, CoordinateSystemPtr const&,
                                              FourMomentum const&, Code const&);

  private:
    inline static auto logger_{get_logger("corsika_proposal_HadronicPhotonModel")};
    THadronicModel& heHadronicInteraction_;
    //! threshold for high energy hadronic interaction model. Lab. energy per nucleon
    HEPEnergyType heHadronicModelThresholdLabNN_;
  };
} // namespace corsika::proposal

#include <corsika/detail/modules/proposal/HadronicPhotonModel.inl>