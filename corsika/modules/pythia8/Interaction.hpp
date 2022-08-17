/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <tuple>
#include <boost/filesystem/path.hpp>

#include <corsika/framework/utility/CorsikaData.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>
#include <corsika/modules/pythia8/Pythia8.hpp>

namespace corsika::pythia8 {
  class Interaction : public InteractionProcess<Interaction> {

  public:
    Interaction(
        boost::filesystem::path const& mpiInitFile = corsika_data("Pythia/main184.mpi"),
        bool const print_listing = false);
    ~Interaction();

    //~ void setStable(std::vector<Code> const&);
    //~ void setUnstable(Code const);
    //~ void setStable(Code const);

    bool isValidCoMEnergy(HEPEnergyType const ecm) const {
      return (10_GeV < ecm) && (ecm < 1_PeV);
    }

    bool canInteract(Code const) const;
    //~ void configureLabFrameCollision(Code const, Code const, HEPEnergyType const);

    bool isValid(Code const projectileId, Code const targetId,
                 HEPEnergyType const sqrtS) const;
    /**
     * Returns inelastic AND elastic cross sections.
     *
     * These cross sections must correspond to the process described in doInteraction
     * AND elastic scattering (sigma_tot = sigma_inel + sigma_el). Allowed targets are:
     * nuclei or single nucleons (p,n,hydrogen). This "InelEla" method is used since
     * Sibyll must be useful inside the NuclearInteraction model, which requires that.
     *
     * @param projectile is the Code of the projectile
     * @param target is the Code of the target
     * @param sqrtSnn is the center-of-mass energy (per nucleon pair)
     * @param Aprojectil is the mass number of the projectils, if it is a nucleus
     * @param Atarget is the mass number of the target, if it is a nucleus
     *
     * @return a tuple of: inelastic cross section, elastic cross section
     */
    std::tuple<CrossSectionType, CrossSectionType> getCrossSectionInelEla(
        Code const projectile, Code const target, FourMomentum const& projectileP4,
        FourMomentum const& targetP4) const;

    /**
     * Returns inelastic (production) cross section.
     *
     * This cross section must correspond to the process described in doInteraction.
     * Allowed targets are: nuclei or single nucleons (p,n,hydrogen).
     *
     * @param projectile is the Code of the projectile
     * @param target is the Code of the target
     * @param sqrtSnn is the center-of-mass energy (per nucleon pair)
     * @param Aprojectil is the mass number of the projectils, if it is a nucleus
     * @param Atarget is the mass number of the target, if it is a nucleus
     *
     * @return inelastic cross section
     * elastic cross section
     */
    CrossSectionType getCrossSection(
        Code const projectile, Code const target, FourMomentum const& projectileP4,
        FourMomentum const& targetP4) const; // non-const for now

    /**
     * In this function PYTHIA is called to produce one event. The
     * event is copied (and boosted) into the shower lab frame.
     */
    template <typename TView>
    void doInteraction(TView& output, Code const projectileId, Code const targetId,
                       FourMomentum const& projectileP4, FourMomentum const& targetP4);

    /**
     * return average number of sub-collisions in a nucleus, using the parameterizations
     * of Sjöstrand and Utheim, EPJ C 82 (2022) 1, 21, arXiv:2108.03481 [hep-ph]
     *
     * @param targetId target (nucleus)
     * @param sigTot projectile-nucleon (=proton) cross-secion
     */
    double getAverageSubcollisions(Code targetId, CrossSectionType sigTot) const;

    static std::array constexpr validTargets_{Code::Oxygen, Code::Nitrogen,
                                              Code::Argon,  Code::Hydrogen,
                                              Code::Proton, Code::Neutron};

  private:
    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("pythia");
    //~ bool const internalDecays_ = true;
    int count_ = 0;
    bool const print_listing_ = false;
    Pythia8::Pythia pythiaMain_;
    Pythia8::Pythia mutable pythiaColl_; // Pythia's getSigma...() are not marked const...
    double const probSD_ =
        0.3; // Fraction of single diffractive events beyond first collision in nucleus.
    HEPEnergyType const eMaxLab_ = 1e18_eV;
    HEPEnergyType const eKinMinLab_ = 0.2_GeV;
  };

} // namespace corsika::pythia8

#include <corsika/detail/modules/pythia8/Interaction.inl>
