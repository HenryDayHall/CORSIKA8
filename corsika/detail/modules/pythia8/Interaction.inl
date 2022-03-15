/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <tuple>

#include <fmt/core.h>

#include <corsika/modules/pythia8/Interaction.hpp>

#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>

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
    readString("SoftQCD:all = on");
    readString("LowEnergyQCD:all = on");
    readString("MultipartonInteraction:reuseInit = 3");
    readString(fmt::format("MultipartonInteraction:initFile = {}", corsika_data("pythia8/MPI_init_file")));
    readString("Beam:allowVariableEnergy = on");
    readString("Beam:allowIDASwitch = on");
    readString("Beam:frameType = 3"); // no special frame, need to specify full 3-mom
    readString("Check:epTolErr = 0.1"); // relax energy-monetum conservation, copied from Pythia 8 main183.cc

    // we can't test this block, LCOV_EXCL_START
    if (!Pythia8::Pythia::init()) {
      throw std::runtime_error("Pythia::Interaction: Initialization failed!");
    }
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

  inline bool Interaction::isValid(Code const projectileId, Code const targetId,
                                   HEPEnergyType const sqrtS) const {
    // TODO: rethink this function. Pythia can do much more now.
    return true;
  }

  inline void Interaction::configureLabFrameCollision(Code const projectileId,
                                                      Code const targetId,
                                                      MomentumVector const& projectileMom,
                                                      MomentumVector const& targetMom) {
    // Pythia configuration of the current event
    // very clumsy. I am sure this can be done better..

    // set beam
    // beam id for pythia
    auto const pdgBeam = static_cast<int>(get_PDG(projectileId));
    
    // set target
    auto pdgTarget = static_cast<int>(get_PDG(targetId));
    // replace hydrogen with proton, otherwise pythia goes into heavy ion mode!
    if (targetId == Code::Hydrogen)
        pdgTarget = static_cast<int>(get_PDG(Code::Proton));
    
    setBeamIDs(pdgBeam, pdgTarget);
    
    auto const projMomGeV = projectileMom.getComponents() * (1/1_GeV);
    auto const tarMomGeV = targetMom.getComponents() * (1/1_GeV);
    
    setKinematics(projMomGeV[0], projMomGeV[1], projMomGeV[2],
                  tarMomGeV[0], tarMomGeV[1], tarMomGeV[2]);
    
    // initialize this config
    // we can't test this block, LCOV_EXCL_START
    if (!Pythia8::Pythia::init())
      throw std::runtime_error("Pythia::Interaction: Initialization failed!");
    // LCOV_EXCL_STOP
  }

  inline bool Interaction::canInteract(Code const pCode) const {
    return true; // TODO: implement this
  }

  CrossSectionType
  Interaction::getCrossSection(Code const projectileId, Code const targetId,
                                      FourMomentum const& projectileP4,
                                      FourMomentum const& targetP4) const {

    HEPEnergyType const CoMenergy = (projectileP4 + targetP4).getNorm();

    if (!isValid(projectileId, targetId, CoMenergy)) {
      return {CrossSectionType::zero(), CrossSectionType::zero()};
    }

    // input particle PDG
    auto const pdgCodeBeam = static_cast<int>(get_PDG(projectileId));
    auto const pdgCodeTarget = static_cast<int>(get_PDG(targetId));
    double const ecm_GeV = CoMenergy * (1 / 1_GeV);
    
    auto const sigTot = pythia.getSigmaTotal(pdgCodeBeam, 2212, ecm_GeV) * 1_mb;
    auto const sigEla = pythia.getSigmaPartial(pdgCodeBeam, 2212, evm_GeV, 2) * 1_mb;
    auto const sigProd = sigTot - sigEla;
    
    if (is_nucleus(targetId)) {
    // avg. no. of sub-collisions, from 2108.03481, for Nitrogen target...
    // TODO: generalize
      double const nCollAvg = (sigTot < 31._mb) ? 1. + (0.017 / 1_mb) * sigTot
                      : 1.2 + (0.0105 / 1_mb) * sigTot;
                      
      // TODO: is this valid also for production XS? Paper says total XS...
      return get_nucleus_A(targetId) * sigProd / nCollAvg;
    } else {
      return sigProd;
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

    if (!isValid(projectileId, targetId, sqrtS)) {
      throw std::runtime_error("invalid target,projectile,energy combination.");
    }

    // position and time of interaction
    Point const& pOrig = projectile.getPosition();
    TimeType const tOrig = projectile.getTime();

    CORSIKA_LOG_DEBUG("Interaction: ebeam lab: {} GeV", eProjectileLab / 1_GeV);

    CORSIKA_LOG_DEBUG("Interaction: position of interaction: ", pOrig.getCoordinates());
    CORSIKA_LOG_DEBUG("Interaction: time: {}", tOrig);

    CORSIKA_LOG_DEBUG(
        "Interaction: "
        " doInteraction: E(GeV): {}"
        " Ecm(GeV): {}",
        eProjectileLab / 1_GeV, sqrtS / 1_GeV);

    count_++;
    configureLabFrameCollision(projectileId, targetId, projectileP4.getSpacelikeComponents(), targetP4.getSpacelikeComponents());

    CrossSectionType const sigmaHp = pythiaColl.getSigmaTotal(idNow, 2212, eCMNow);

    // create event in pytia. LCOV_EXCL_START: we don't validate pythia8 internals
    if (!Pythia8::Pythia::next())
      throw std::runtime_error("Pythia::DoInteraction: failed!");
    // LCOV_EXCL_STOP

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

      HEPEnergyType const mass = get_mass(pyId);
      HEPEnergyType const Ekin = sqrt(pyPlab.getSquaredNorm() + mass * mass) - mass;

      // add to corsika stack
      auto pnew =
          projectile.addSecondary(std::make_tuple(pyId, Ekin, pyPlab.normalized()));

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
