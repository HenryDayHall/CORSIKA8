/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/modules/qgsjetII/ParticleConversion.hpp>
#include <corsika/framework/utility/CorsikaData.hpp>

#include <boost/filesystem/path.hpp>

#include <qgsjet-II-04.hpp>
#include <string>

namespace corsika::qgsjetII {

  class Interaction : public corsika::InteractionProcess<Interaction> {

  public:
    Interaction(boost::filesystem::path dataPath = corsika_data("QGSJetII"));
    ~Interaction();

    bool wasInitialized() { return initialized_; }
    unsigned int getMaxTargetMassNumber() const { return maxMassNumber_; }
    bool isValidTarget(corsika::Code TargetId) const {
      return is_nucleus(TargetId) && (get_nucleus_A(TargetId) < maxMassNumber_);
    }

    CrossSectionType getCrossSection(const Code, const Code, const HEPEnergyType,
                                     const unsigned int Abeam = 0,
                                     const unsigned int Atarget = 0) const;

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&) const;

    /**
       In this function QGSJETII is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

    template <typename TSecondaryView>
    void doInteraction(TSecondaryView&);

  private:
    int count_ = 0;
    bool initialized_ = false;
    QgsjetIIHadronType alternate_ =
        QgsjetIIHadronType::PiPlusType; // for pi0, rho0 projectiles

    corsika::default_prng_type& rng_ =
        corsika::RNGManager<>::getInstance().getRandomStream("qgsjet");
    static unsigned int constexpr maxMassNumber_ = 208;
  };

} // namespace corsika::qgsjetII

#include <corsika/detail/modules/qgsjetII/Interaction.inl>
