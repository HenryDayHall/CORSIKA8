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
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/utility/CorsikaData.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/multi_array.hpp>

#include <array>
#include <utility>
#include <string>

namespace corsika::urqmd {

  class UrQMD : public InteractionProcess<UrQMD> {
  public:
    /**
     * @param path Location of UrQMD XS data file
     * @param retryFlag Internal UrQMD flag for retrying interaction in case of empty
     * event, 0 means retry
     */
    UrQMD(boost::filesystem::path const path = corsika_data("UrQMD/UrQMD-1.3.1-xs.dat"),
          int const retryFlag = 0);

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const&) const;

    CrossSectionType getTabulatedCrossSection(Code, Code, HEPEnergyType) const;

    template <typename TParticle>
    CrossSectionType getCrossSection(TParticle const&, Code) const;

    template <typename TView>
    void doInteraction(TView&);

    bool canInteract(Code) const;

    void blob(int) {}

    static CrossSectionType getCrossSection(Code, Code, HEPEnergyType, int);

  private:
    void readXSFile(boost::filesystem::path);

    // data members
    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("urqmd");
    std::uniform_int_distribution<int> booleanDist_{0, 1};
    int iflb_; //! // flag for retrying interaction in case of empty event, 0 means retry
    boost::multi_array<CrossSectionType, 3> xs_interp_support_table_;
  };

  /**
   * convert CORSIKA code to UrQMD code tuple
   *
   * In the current implementation a detour via the PDG code is made.
   */
  std::pair<int, int> convertToUrQMD(Code);
  Code convertFromUrQMD(int vItyp, int vIso3);

} // namespace corsika::urqmd

#include <corsika/detail/modules/urqmd/UrQMD.inl>
