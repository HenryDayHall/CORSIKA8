/*
 * (c) Copyright 2024 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <boost/filesystem/path.hpp>

namespace corsika::pythia8 {

  inline NeutrinoInteraction::NeutrinoInteraction(bool const& handleNC,
                                                  bool const& handleCC,
                                                  bool const print_listing)
      : print_listing_(print_listing)
      , handle_nc_(handleNC)
      , handle_cc_(handleCC)
      , pythiaMain_{CORSIKA_Pythia8_XML_DIR, false} {

    pythiaMain_.setRndmEnginePtr(std::make_shared<corsika::pythia8::Random>());
  }

  inline NeutrinoInteraction::~NeutrinoInteraction() {
    CORSIKA_LOG_INFO("Pythia::NeutrinoInteraction n= {}", count_);
  }

  template <class TView>
  void NeutrinoInteraction::doInteraction(TView& view, Code const projectileId,
                                          Code const targetId,
                                          FourMomentum const& projectileP4,
                                          FourMomentum const& targetP4) {

    CORSIKA_LOG_INFO("Primary {} - {} interaction with E_nu = {} GeV", projectileId,
                     targetId, projectileP4.getTimeLikeComponent() / 1_GeV);
    CORSIKA_LOG_INFO("configure Pythia for primary neutrino interactions. NC={}, CC={}",
                     handle_nc_, handle_cc_);
    if (!handle_nc_ && !handle_cc_) {
      CORSIKA_LOG_ERROR(
          "no neutrino interaction channel configured! Select either NC, CC or both!");
      throw std::runtime_error("Configuration error!");
    }
    CORSIKA_LOG_INFO("minimal Q2 in DIS: {} GeV2", minQ2_ / 1_GeV / 1_GeV);

    if (!isValid(projectileId, targetId, projectileP4, targetP4)) {
      CORSIKA_LOG_ERROR("wrong projectile, target or energy configuration!");
      throw std::runtime_error("Configuration error!");
    }
    // sample nucleon from nucleus A,Z
    double const fProtons = get_nucleus_Z(targetId) / double(get_nucleus_A(targetId));
    double const fNeutrons = 1. - fProtons;
    std::discrete_distribution<int> nucleonChannelDist{fProtons, fNeutrons};
    corsika::default_prng_type& rng =
        corsika::RNGManager<>::getInstance().getRandomStream("pythia");
    Code const nucleonId = (nucleonChannelDist(rng) ? Code::Neutron : Code::Proton);
    int const idTarget = static_cast<int>(get_PDG(nucleonId));
    CORSIKA_LOG_INFO("selected {} target", nucleonId);
    double const eTarget = get_mass(nucleonId) / 1_GeV;

    // set projectile
    double const eElectron = projectileP4.getTimeLikeComponent() / 1_GeV; // 270.5;
    double const Q2min = minQ2_ / 1_GeV / 1_GeV;
    int const idProjectile = static_cast<int>(get_PDG(projectileId));

    // Set up incoming beams, for frame with unequal beam energies.
    // projectile is along -z
    pythiaMain_.readString("Beams:frameType = 2");
    // BeamA = nucleon.(+z)
    pythiaMain_.settings.mode("Beams:idB", idTarget);
    pythiaMain_.settings.parm("Beams:eA", eTarget);
    // BeamB = neutrino (-z direction)
    pythiaMain_.settings.mode("Beams:idB", idProjectile);
    pythiaMain_.settings.parm("Beams:eB", eElectron);

    // Set up DIS process within some phase space.
    // Neutral current (with gamma/Z interference).
    if (handle_nc_) pythiaMain_.readString("WeakBosonExchange:ff2ff(t:gmZ) = on");
    // Charged current.
    if (handle_cc_) pythiaMain_.readString("WeakBosonExchange:ff2ff(t:W) = on");
    // Phase-space cut: minimal Q2 of process.
    pythiaMain_.settings.parm("PhaseSpace:Q2Min", Q2min);

    // Set dipole recoil on. Necessary for DIS + shower.
    pythiaMain_.readString("SpaceShower:dipoleRecoil = on");

    // Allow emissions up to the kinematical limit,
    // since rate known to match well to matrix elements everywhere.
    pythiaMain_.readString("SpaceShower:pTmaxMatch = 2");

    // QED radiation off lepton not handled yet by the new procedure.
    pythiaMain_.readString("PDF:lepton = off");
    pythiaMain_.readString("TimeShower:QEDshowerByL = off");

    // // no Decays to be done by pythiaMain_.
    pythiaMain_.readString("HadronLevel:Decay = off");

    pythiaMain_.readString("Stat:showProcessLevel = off");
    pythiaMain_.readString("Stat:showPartonLevel = off");

    pythiaMain_.readString("Print:quiet = on");
    pythiaMain_.readString("Check:epTolErr = 0.1");
    pythiaMain_.readString("Check:epTolWarn = 0.0001");
    pythiaMain_.readString("Check:mTolErr = 0.01");

    pythiaMain_.init();

    // References to the event record
    Pythia8::Event& eventMain = pythiaMain_.event;

    COMBoost const labFrameBoost{targetP4.getSpaceLikeComponents(), get_mass(targetId)};
    auto const proj4MomLab = labFrameBoost.toCoM(projectileP4);
    auto const& rotCS = labFrameBoost.getRotatedCS();

    if (!pythiaMain_.next()) {
      throw std::runtime_error("Pythia neutrino collision failed ");
    } else {
      CORSIKA_LOG_INFO("pythia neutrino interaction done!");
    }

    MomentumVector Plab_final{labFrameBoost.getOriginalCS()};
    auto Elab_final = HEPEnergyType::zero();
    CORSIKA_LOG_INFO("particles generated in neutrino interaction:");
    for (int i = 0; i < eventMain.size(); ++i) {
      auto const& p8p = eventMain[i];
      if (p8p.isFinal()) {
        try {
          auto const volatile id = static_cast<PDGCode>(p8p.id());
          auto const pyId = convert_from_PDG(id);

          MomentumVector const pyPlab(
              rotCS, {p8p.px() * 1_GeV, p8p.py() * 1_GeV, p8p.pz() * 1_GeV});
          auto const pyP = labFrameBoost.fromCoM(FourVector{p8p.e() * 1_GeV, pyPlab});

          HEPEnergyType const mass = get_mass(pyId);
          HEPEnergyType const Ekin =
              sqrt(pyP.getSpaceLikeComponents().getSquaredNorm() + mass * mass) - mass;

          // add to corsika stack
          auto pnew = view.addSecondary(std::make_tuple(pyId, Ekin, pyPlab.normalized()));

          CORSIKA_LOG_INFO("id = {}, E = {} GeV, p = {} GeV", pyId, Ekin / 1_GeV,
                           pyPlab.getComponents() / 1_GeV);
          Plab_final += pnew.getMomentum();
          Elab_final += pnew.getEnergy();
        }
        // irreproducible in tests, LCOV_EXCL_START
        catch (std::out_of_range const& ex) {
          CORSIKA_LOG_CRITICAL("Pythia ID {} unknown in C8", p8p.id());
          throw ex;
        }
        // LCOV_EXCL_STOP
      }
    }

    CORSIKA_LOG_DEBUG(
        "conservation (all GeV): "
        "Elab_final= {}"
        ", Plab_final= {}",
        Elab_final / 1_GeV, (Plab_final / 1_GeV).getComponents());

    count_++;
  }

} // namespace corsika::pythia8