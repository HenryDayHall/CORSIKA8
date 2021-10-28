/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/pythia8/Interaction.hpp>

#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <tuple>

namespace corsika::pythia8 {

  inline Interaction::~Interaction() {
    CORSIKA_LOG_INFO("Pythia::Interaction n= {}", count_);
  }

  inline Interaction::Interaction(bool const print_listing)
      : Pythia8::Pythia(CORSIKA_Pythia8_XML_DIR)
      , print_listing_(print_listing) {

    CORSIKA_LOG_INFO("Configuring Pythia8 from: {}", CORSIKA_Pythia8_XML_DIR);

    // initialize Pythia

    // reduce output from pythia if set to "on"
    Pythia8::Pythia::readString("Print:quiet = on");
    // check if data in particle data file is minimally consistent. Very verbose! set to
    // "off"! we do not change the basic file provided by pythia.
    Pythia8::Pythia::readString("Check:particleData = off");
    Pythia8::Pythia::readString("Check:event = on");             // default: on
    Pythia8::Pythia::readString("Check:levelParticleData = 12"); // 1 is default
    /** \TODO: proper process initialization for MinBias needed, see
        also Issue https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/issues/369 **/
    Pythia8::Pythia::readString("HardQCD:all = on");
    Pythia8::Pythia::readString("ProcessLevel:resonanceDecays = off");

    // we can't test this block, LCOV_EXCL_START
    if (!Pythia8::Pythia::init())
      throw std::runtime_error("Pythia::Interaction: Initialization failed!");
    // LCOV_EXCL_STOP

    // any decays in pythia? if yes need to define which particles
    if (internalDecays_) {
      // define which particles are passed to corsika, i.e. which particles make it into
      // history even very shortlived particles like charm or pi0 are of interest here
      std::vector<Code> const HadronsWeWantTrackedByCorsika = {
          Code::PiPlus, Code::PiMinus, Code::Pi0,        Code::KMinus,     Code::KPlus,
          Code::K0Long, Code::K0Short, Code::SigmaPlus,  Code::SigmaMinus, Code::Lambda0,
          Code::Xi0,    Code::XiMinus, Code::OmegaMinus, Code::DPlus,      Code::DMinus,
          Code::D0,     Code::D0Bar};

      Interaction::setStable(HadronsWeWantTrackedByCorsika);
    }

    // basic initialization of cross section routines
    sigma_.init(&(Pythia8::Pythia::info), Pythia8::Pythia::settings,
                &(Pythia8::Pythia::particleData), &(Pythia8::Pythia::rndm));
  }

  inline void Interaction::setStable(std::vector<Code> const& particleList) {
    for (auto p : particleList) Interaction::setStable(p);
  }

  inline void Interaction::setUnstable(Code const pCode) {
    CORSIKA_LOG_DEBUG("Pythia::Interaction: setting {} unstable..", pCode);
    Pythia8::Pythia::particleData.mayDecay(static_cast<int>(get_PDG(pCode)), true);
  }

  inline void Interaction::setStable(Code const pCode) {
    CORSIKA_LOG_DEBUG("Pythia::Interaction: setting {} stable..", pCode);
    Pythia8::Pythia::particleData.mayDecay(static_cast<int>(get_PDG(pCode)), false);
  }

  inline void Interaction::isValid(Code const projectileId, Code const targetId,
                                   HEPEnergyType const sqrtS) const {

    if ((10_GeV > sqrtS) || (sqrtS > 1_PeV)) {
      throw std::runtime_error("energy out of bounds for PYTHIA");
    }

    if (targetId != Code::Hydrogen && targetId != Code::Neutron &&
        targetId != Code::Proton) {
      throw std::runtime_error("wrong target for PYTHIA");
    }

    if (is_nucleus(projectileId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by PYTHIA!");
    }

    if (!canInteract(projectileId)) {
      throw std::runtime_error("Projectile not supported by PYTHIA!");
    }
  }

  inline void Interaction::configureLabFrameCollision(Code const projectileId,
                                                      Code const targetId,
                                                      HEPEnergyType const BeamEnergy) {
    // Pythia configuration of the current event
    // very clumsy. I am sure this can be done better..

    // set beam
    // beam id for pythia
    auto const pdgBeam = static_cast<int>(get_PDG(projectileId));
    std::stringstream stBeam;
    stBeam << "Beams:idA = " << pdgBeam;
    Pythia8::Pythia::readString(stBeam.str());
    // set target
    auto pdgTarget = static_cast<int>(get_PDG(targetId));
    // replace hydrogen with proton, otherwise pythia goes into heavy ion mode!
    if (targetId == Code::Hydrogen) pdgTarget = static_cast<int>(get_PDG(Code::Proton));
    std::stringstream stTarget;
    stTarget << "Beams:idB = " << pdgTarget;
    Pythia8::Pythia::readString(stTarget.str());
    // set frame to lab. frame
    Pythia8::Pythia::readString("Beams:frameType = 2");
    // set beam energy
    double const Elab = BeamEnergy / 1_GeV;
    std::stringstream stEnergy;
    stEnergy << "Beams:eA = " << Elab;
    Pythia8::Pythia::readString(stEnergy.str());
    // target at rest
    Pythia8::Pythia::readString("Beams:eB = 0.");
    // initialize this config

    // we can't test this block, LCOV_EXCL_START
    if (!Pythia8::Pythia::init())
      throw std::runtime_error("Pythia::Interaction: Initialization failed!");
    // LCOV_EXCL_STOP
  }

  inline bool Interaction::canInteract(Code const pCode) const {
    return pCode == Code::Proton || pCode == Code::Neutron || pCode == Code::AntiProton ||
           pCode == Code::AntiNeutron || pCode == Code::PiMinus || pCode == Code::PiPlus;
  }

  inline std::tuple<CrossSectionType, CrossSectionType>
  Interaction::getCrossSectionInelEla(Code const projectileId, Code const targetId,
                                      FourMomentum const& projectileP4,
                                      FourMomentum const& targetP4) const {

    HEPEnergyType const CoMenergy = (projectileP4 + targetP4).getNorm();

    isValid(projectileId, targetId, CoMenergy); // throws

    // input particle PDG
    auto const pdgCodeBeam = static_cast<int>(get_PDG(projectileId));
    auto const pdgCodeTarget = static_cast<int>(get_PDG(targetId));
    double const ecm = CoMenergy / 1_GeV;

    //! @todo: remove this const_cast, when Pythia8 becomes const-correct! CHECK!
    Pythia8::SigmaTotal& sigma = *const_cast<Pythia8::SigmaTotal*>(&sigma_);

    // calculate cross section
    sigma.calc(pdgCodeBeam, pdgCodeTarget, ecm);
    if (sigma.hasSigmaTot()) {
      double const sigEla = sigma.sigmaEl();
      double const sigProd = sigma.sigmaTot() - sigEla;

      return std::make_tuple(sigProd * (1_fm * 1_fm), sigEla * (1_fm * 1_fm));

    } else {
      // we can't test pythia8 internals, LCOV_EXCL_START
      throw std::runtime_error("pythia cross section init failed");
      // we can't test pythia8 internals, LCOV_EXCL_STOP
    }
  }

  template <class TView>
  inline void Interaction::doInteraction(TView& view, Code const projectileId,
                                         Code const targetId,
                                         FourMomentum const& projectileP4,
                                         FourMomentum const& targetP4) {

    auto projectile = view.getProjectile();

    CORSIKA_LOG_DEBUG(
        "Pythia::Interaction: "
        "doInteraction: {} interaction? ",
        projectileId, corsika::pythia8::Interaction::canInteract(projectileId));

    // define system
    auto const sqrtS2 = (projectileP4 + targetP4).getNormSqr();
    HEPEnergyType const sqrtS = sqrt(sqrtS2);
    HEPEnergyType const eProjectileLab = (sqrtS2 - static_pow<2>(get_mass(projectileId)) -
                                          static_pow<2>(get_mass(targetId))) /
                                         (2 * get_mass(targetId));

    isValid(projectileId, targetId, sqrtS); // throws

    // position and time of interaction
    Point pOrig = projectile.getPosition();
    TimeType tOrig = projectile.getTime();

    CORSIKA_LOG_DEBUG("Interaction: ebeam lab: {} GeV", eProjectileLab / 1_GeV);

    // define target kinematics in lab frame
    // define boost to and from CoM frame
    // CoM frame definition in Pythia projectile: +z
    COMBoost const boost(projectileP4, constants::nucleonMass);
    auto const& labCS = boost.getOriginalCS();

    CORSIKA_LOG_DEBUG("Interaction: position of interaction: ", pOrig.getCoordinates());
    CORSIKA_LOG_DEBUG("Interaction: time: {}", tOrig);

    CORSIKA_LOG_DEBUG(
        "Interaction: "
        " doInteraction: E(GeV): {}"
        " Ecm(GeV): {}",
        eProjectileLab / 1_GeV, sqrtS / 1_GeV);

    count_++;

    configureLabFrameCollision(projectileId, targetId, eProjectileLab);

    // create event in pytia. LCOV_EXCL_START: we don't validate pythia8 internals
    if (!Pythia8::Pythia::next())
      throw std::runtime_error("Pythia::DoInteraction: failed!");
    // LCOV_EXCL_STOP

    // link to pythia stack
    Pythia8::Event& event = Pythia8::Pythia::event;

    // LCOV_EXCL_START, we don't validate pythia8 internals
    if (print_listing_) {
      // print final state
      event.list();
    }
    // LCOV_EXCL_STOP

    MomentumVector Plab_final(labCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    HEPEnergyType Elab_final = 0_GeV;
    for (int i = 0; i < event.size(); ++i) {
      Pythia8::Particle const& p8p = event[i];
      // skip particles that have decayed in pythia
      if (!p8p.isFinal()) continue;

      auto const pyId = convert_from_PDG(static_cast<PDGCode>(p8p.id()));

      MomentumVector const pyPlab(labCS,
                                  {p8p.px() * 1_GeV, p8p.py() * 1_GeV, p8p.pz() * 1_GeV});

      // add to corsika stack
      auto pnew = projectile.addSecondary(std::make_tuple(pyId, pyPlab, pOrig, tOrig));

      Plab_final += pnew.getMomentum();
      Elab_final += pnew.getEnergy();
    }

    CORSIKA_LOG_DEBUG(
        "conservation (all GeV): "
        "Elab_final= {}"
        ", Plab_final= {}",
        Elab_final / 1_GeV, (Plab_final / 1_GeV).getComponents());
  }

} // namespace corsika::pythia8
