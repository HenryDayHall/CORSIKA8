/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/epos/InteractionModel.hpp>
#include <corsika/modules/epos/EposStack.hpp>

#include <corsika/framework/geometry/Point.hpp>

#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/utility/CorsikaData.hpp>

#include <epos.hpp>

#include <string>
#include <tuple>
#include <cmath>
#include "corsika/framework/core/ParticleProperties.hpp"

namespace corsika::epos {

  inline InteractionModel::InteractionModel(std::string const& dataPath,
                                            bool const epos_printout_on)
      : data_path_(dataPath)
      , epos_listing_(epos_printout_on) {
    // initialize Eposlhc
    if (!isInitialized_) {
      isInitialized_ = true;
      if (dataPath == "") {
        data_path_ = (std::string(corsika_data("EPOS").c_str()) + "/").c_str();
      }
      initialize();
    }
    setParticlesStable();
  }

  inline void InteractionModel::setParticlesStable() const {
    CORSIKA_LOGGER_DEBUG(logger_,
                         "set all particles known to CORSIKA stable inside EPOS..");
    for (auto& p : get_all_particles()) {
      if (!is_hadron(p)) continue;
      int const eid = convertToEposRaw(p);
      if (eid != 0) {
        ::epos::nodcy_.nrnody = ::epos::nodcy_.nrnody + 1;
        ::epos::nodcy_.nody[::epos::nodcy_.nrnody - 1] = eid;
      } else {
        CORSIKA_LOG_WARN(
            "particle conversion Corsika-->Epos not known for {}. Using {}. Setting "
            "unstable in Epos!",
            p, eid);
      }
    }
  }

  inline bool InteractionModel::isValid(Code const projectileId, Code const targetId,
                                        HEPEnergyType const sqrtS) const {
    //! eposlhc only accepts nuclei with X<=A<=Y as targets, or protons aka Hydrogen or
    //! neutrons (p,n == nucleon)
    if (!is_nucleus(targetId) && targetId != Code::Neutron && targetId != Code::Proton) {
      return false;
    }
    if (is_nucleus(targetId) &&
        (get_nucleus_A(targetId) >= get_nucleus_A(maxNucleus_))) {
      return false;
    }
    if ((minEnergyCoM_ > sqrtS) || (sqrtS > maxEnergyCoM_)) { return false; }
    if (!epos::canInteract(projectileId)) { return false; }
    return true;
  }

  inline void InteractionModel::initialize() const {

    CORSIKA_LOGGER_DEBUG(logger_, "initializing...");

    // corsika7 ini
    int iarg = 0;
    ::epos::aaset_(iarg);

    // debug output settings
    ::epos::prnt1_.ish = 0;
    ::epos::prnt3_.iwseed = 0; // 1: printout seeds, 0: off
    ::epos::files_.ifch = 6;   // output unit, 6: screen

    // dummy set seeds for random number generator in epos. need to fool epos checks...
    // we will use external generator
    ::epos::cseed_.seedi = 1;
    ::epos::cseed_.seedj = 1;
    ::epos::cseed_.seedc = 1;

    ::epos::enrgy_.egymin = minEnergyCoM_ / 1_GeV; // 6.;
    ::epos::enrgy_.egymax = maxEnergyCoM_ / 1_GeV; // 2.e6;

    ::epos::lhcparameters_();

    ::epos::hadr6_.isigma = 0; // do not show cross section
    ::epos::hadr6_.isetcs = 3; /*  !option to obtain pomeron parameters
      ! 0.....determine parameters but do not use Kfit
      ! 1.....determine parameters and use Kfit
      ! else..get from table
      !         should be sufficiently detailed
      !          say iclegy1=1,iclegy2=99
      !         table is always done, more or less detailed!!!
      !and option to use cross section tables
      ! 2....tabulation
      ! 3....simulation
                               */
    ::epos::cjinti_.ionudi =
        1; // !include quasi elastic events but strict calculation of xs
    ::epos::cjinti_.iorsce = 0; // !color exchange turned on(1) or off(0)
    ::epos::cjinti_.iorsdf = 3; //  !droplet formation turned on(>0) or off(0)
    ::epos::cjinti_.iorshh = 0; //    !other hadron-hadron int. turned on(1) or off(0)

    ::epos::othe1_.istore = 0; // do not produce epos output file
    ::epos::nucl6_.infragm =
        2; // 0: keep free nucleons in fragmentation,1: one fragment, 2: fragmentation

    ::epos::othe2_.iframe = 11; // cms frame

    // set paths to tables in corsika data
    ::epos::datadir BASE(data_path_);
    strcpy(::epos::fname_.fnnx, BASE.data);
    ::epos::nfname_.nfnnx = BASE.length;

    ::epos::datadir TL(data_path_ + "epos.initl");
    strcpy(::epos::fname_.fnii, TL.data);
    ::epos::nfname_.nfnii = TL.length;

    ::epos::datadir EV(data_path_ + "epos.iniev");
    strcpy(::epos::fname_.fnie, EV.data);
    ::epos::nfname_.nfnie = EV.length;

    ::epos::datadir RJ(data_path_ + "epos.inirj"); // lhcparameters adds ".lhc"
    strcpy(::epos::fname_.fnrj, RJ.data);
    ::epos::nfname_.nfnrj = RJ.length;

    ::epos::datadir CS(data_path_ + "epos.inics"); // lhcparameters adds ".lhc"
    strcpy(::epos::fname_.fncs, CS.data);
    ::epos::nfname_.nfncs = CS.length;

    // initializes maximum energy and mass
    initializeEventCoM(
        maxNucleus_, get_nucleus_A(maxNucleus_), get_nucleus_Z(maxNucleus_), maxNucleus_,
        get_nucleus_A(maxNucleus_), get_nucleus_Z(maxNucleus_), maxEnergyCoM_);
  }

  inline void InteractionModel::initializeEventCoM(Code const idBeam, int const iBeamA,
                                                   int const iBeamZ, Code const idTarget,
                                                   int const iTargetA, int const iTargetZ,
                                                   HEPEnergyType const EcmNN) const {
    CORSIKA_LOGGER_TRACE(logger_,
                         "initialize event in CoM frame!"
                         " Ecm={}",
                         EcmNN);
    ::epos::lept1_.engy = -1.;
    ::epos::enrgy_.ecms = -1.;
    ::epos::enrgy_.elab = -1.;
    ::epos::enrgy_.ekin = -1.;
    ::epos::hadr1_.pnll = -1.;

    ::epos::enrgy_.ecms = EcmNN / 1_GeV; // -> c.m.s. frame

    CORSIKA_LOGGER_TRACE(logger_, "inside EPOS: Ecm={}, Elab={}", ::epos::enrgy_.ecms,
                         ::epos::enrgy_.elab);

    configureParticles(idBeam, iBeamA, iBeamZ, idTarget, iTargetA, iTargetZ);
    ::epos::ainit_();
  }

  inline void InteractionModel::configureParticles(Code const idBeam, int const iBeamA,
                                                   int const iBeamZ, Code const idTarget,
                                                   int const iTargetA,
                                                   int const iTargetZ) const {
    CORSIKA_LOGGER_TRACE(logger_,
                         "setting "
                         "Beam={}, "
                         "BeamA={}, "
                         "BeamZ={}, "
                         "Target={}, "
                         "TargetA={}, "
                         "TargetZ={} ",
                         idBeam, iBeamA, iBeamZ, idTarget, iTargetA, iTargetZ);

    if (is_nucleus(idBeam)) {
      ::epos::hadr25_.idprojin = convertToEposRaw(Code::Proton);
      ::epos::nucl1_.laproj = iBeamZ;
      ::epos::nucl1_.maproj = iBeamA;
    } else {
      ::epos::hadr25_.idprojin = convertToEposRaw(idBeam);
      ::epos::nucl1_.laproj = -1;
      ::epos::nucl1_.maproj = 1;
    }

    if (is_nucleus(idTarget)) {
      ::epos::hadr25_.idtargin = convertToEposRaw(Code::Proton);
      ::epos::nucl1_.matarg = iTargetA;
      ::epos::nucl1_.latarg = iTargetZ;
    } else if (idTarget == Code::Proton || idTarget == Code::Hydrogen) {
      ::epos::hadr25_.idtargin = convertToEposRaw(Code::Proton);
      ::epos::nucl1_.matarg = 1;
      ::epos::nucl1_.latarg = -1;
    } else if (idTarget == Code::Neutron) {
      ::epos::hadr25_.idtargin = convertToEposRaw(Code::Neutron);
      ::epos::nucl1_.matarg = 1;
      ::epos::nucl1_.latarg = -1;
    }

    CORSIKA_LOGGER_TRACE(logger_,
                         "inside EPOS: "
                         "Id beam={}, "
                         "Z beam={}, "
                         "A beam={}, "
                         "XS beam={}, "
                         "Id target={}, "
                         "Z target={}, "
                         "A target={}, "
                         "XS target={} ",
                         ::epos::hadr25_.idprojin, ::epos::nucl1_.laproj,
                         ::epos::nucl1_.maproj, ::epos::had10_.iclpro,
                         ::epos::hadr25_.idtargin, ::epos::nucl1_.latarg,
                         ::epos::nucl1_.matarg, ::epos::had10_.icltar);
  }

  inline InteractionModel::~InteractionModel() {
    CORSIKA_LOGGER_DEBUG(logger_, "n={} ", count_);
  }

  inline std::tuple<CrossSectionType, CrossSectionType>
  InteractionModel::calcCrossSectionCoM(Code const BeamId, int const BeamA,
                                        int const BeamZ, Code const TargetId,
                                        int const TargetA, int const TargetZ,
                                        const HEPEnergyType EnergyCOM) const {
    CORSIKA_LOGGER_DEBUG(logger_,
                         "calcCrossSection: input:"
                         " beamId={}, beamA={}, beamZ={}"
                         " target={}, targetA={}, targetZ={}"
                         " Ecm={:4.3f} GeV,",
                         BeamId, BeamA, BeamZ, TargetId, TargetA, TargetZ,
                         EnergyCOM / 1_GeV);

    const int iBeam = epos::getEposXSCode(
        BeamId); // 0 (can not interact, 1: proton-like, 2: pion-like, 3:kaon-like)

    CORSIKA_LOGGER_TRACE(logger_,
                         "projectile cross section type={} "
                         "(0: cannot interact, 1:pion, 2:baryon, 3:kaon)",
                         iBeam);
    // reset beam particle // (1: pion-like, 2: proton-like, 3:kaon-like)
    if (iBeam == 1)
      initializeEventCoM(Code::PiPlus, BeamA, BeamZ, TargetId, TargetA, TargetZ,
                         EnergyCOM);
    else if (iBeam == 2)
      initializeEventCoM(Code::Proton, BeamA, BeamZ, TargetId, TargetA, TargetZ,
                         EnergyCOM);
    else if (iBeam == 3)
      initializeEventCoM(Code::KPlus, BeamA, BeamZ, TargetId, TargetA, TargetZ,
                         EnergyCOM);

    double sigProd, sigEla = 0;
    float sigTot1, sigProd1, sigCut1 = 0;
    if (!is_nucleus(TargetId) && !is_nucleus(BeamId)) {
      sigProd = ::epos::hadr5_.sigine;
      sigEla = ::epos::hadr5_.sigela;
    } else {
      // calculate from model, SLOW:
      float sigQEla1 = 0; // target fragmentation/excitation
      ::epos::crseaaepos_(sigTot1, sigProd1, sigCut1, sigQEla1);
      sigProd = sigProd1;
      // sigEla not properly defined here
    }
    CORSIKA_LOGGER_DEBUG(logger_,
                         "calcCrossSectionCoM: output:"
                         " sigProd={} mb,"
                         " sigEla={} mb",
                         sigProd, sigEla);

    return std::make_tuple(sigProd * 1_mb, sigEla * 1_mb);
  }

  inline std::tuple<CrossSectionType, CrossSectionType>
  InteractionModel::readCrossSectionTableLab(Code const BeamId, int const BeamA,
                                             int const BeamZ, Code const TargetId,
                                             HEPEnergyType const EnergyLab) const {
    CORSIKA_LOGGER_DEBUG(logger_,
                         "readCrossSectionTableLab: input: "
                         "beamId={}, "
                         "beamA={}, "
                         "beamZ={} "
                         "targetId={}, "
                         "ELab={:12.2f} GeV,",
                         BeamId, BeamA, BeamZ, TargetId, EnergyLab / 1_GeV);

    // read cross section from epos internal tables
    int Abeam = 0;
    float Ekin = -1;

    if (is_nucleus(BeamId)) {
      Abeam = BeamA;
      // kinetic energy per nucleon
      Ekin = (EnergyLab / Abeam - constants::nucleonMass) / 1_GeV;
    } else {
      ::epos::hadr2_.idproj = convertToEposRaw(BeamId);
      int const iBeam = epos::getEposXSCode(
          BeamId); // 0 (can not interact, 1: pion-like, 2: proton-like, 3:kaon-like)
      CORSIKA_LOGGER_TRACE(logger_,
                           "readCrossSectionTableLab: projectile cross section type={} "
                           "(0: cannot interact, 1:pion, 2:baryon, 3:kaon)",
                           iBeam);

      ::epos::had10_.iclpro = iBeam;
      Abeam = 1;
      Ekin = (EnergyLab - get_mass(BeamId)) / 1_GeV;
    }

    int Atarget = 1;
    if (is_nucleus(TargetId)) { Atarget = get_nucleus_A(TargetId); }

    int iMode = 3; // 0: air, >0 not air

    CORSIKA_LOGGER_TRACE(logger_,
                         "readCrossSectionTableLab: inside Epos "
                         "beamId={}, beamXS={}",
                         ::epos::hadr2_.idproj, ::epos::had10_.iclpro);

    CORSIKA_LOGGER_TRACE(logger_,
                         "readCrossSectionTableLab: calling Epos cross section with"
                         "Ekin = {}, Abeam = {}, Atarget = {}, iMode = {}",
                         Ekin, Abeam, Atarget, iMode);

    // cross section from table, FAST
    float sigProdEpos = ::epos::eposcrse_(Ekin, Abeam, Atarget, iMode);
    // sig-el from analytic calculation, no fast
    float sigElaEpos = ::epos::eposelacrse_(Ekin, Abeam, Atarget, iMode);

    CORSIKA_LOGGER_TRACE(logger_,
                         "readCrossSectionTableLab: result: sigProd = {}, sigEla = {}",
                         sigProdEpos, sigElaEpos);

    return std::make_tuple(sigProdEpos * 1_mb, sigElaEpos * 1_mb);
  }

  inline std::tuple<CrossSectionType, CrossSectionType>
  InteractionModel::getCrossSectionInelEla(Code const projectileId, Code const targetId,
                                           FourMomentum const& projectileP4,
                                           FourMomentum const& targetP4) const {
    auto const sqrtS2 = (projectileP4 + targetP4).getNormSqr();
    auto const sqrtS = sqrt(sqrtS2);

    if (!isValid(projectileId, targetId, sqrtS)) {
      return {CrossSectionType::zero(), CrossSectionType::zero()};
    }
    HEPEnergyType const Elab = (sqrtS2 - static_pow<2>(get_mass(projectileId)) -
                                static_pow<2>(get_mass(targetId))) /
                               (2 * get_mass(targetId));
    int beamA = 1;
    int beamZ = 1;
    if (is_nucleus(projectileId)) {
      beamA = get_nucleus_A(projectileId);
      beamZ = get_nucleus_Z(projectileId);
    }

    CORSIKA_LOGGER_DEBUG(logger_,
                         "getCrossSectionLab: input:"
                         " beamId={}, beamA={}, beamZ={}"
                         " target={}"
                         " ELab={:4.3f} GeV, sqrtS={}",
                         projectileId, beamA, beamZ, targetId, Elab / 1_GeV,
                         sqrtS / 1_GeV);
    return readCrossSectionTableLab(projectileId, beamA, beamZ, targetId, Elab);
  }

  template <typename TSecondaryView>
  inline void InteractionModel::doInteraction(TSecondaryView& view,
                                              Code const projectileId,
                                              Code const targetId,
                                              FourMomentum const& projectileP4,
                                              FourMomentum const& targetP4) {

    count_ = count_ + 1;

    // define nucleon-nucleon center-of-mass frame
    auto const projectileP4NN =
        projectileP4 / (is_nucleus(projectileId) ? get_nucleus_A(projectileId) : 1);
    auto const targetP4NN =
        targetP4 / (is_nucleus(targetId) ? get_nucleus_A(targetId) : 1);
    auto const SNN = (projectileP4NN + targetP4NN).getNormSqr();
    HEPEnergyType const sqrtSNN = sqrt(SNN);
    if (!isValid(projectileId, targetId, sqrtSNN)) {
      throw std::runtime_error("invalid projectile/target/energy combination.");
    }
    HEPEnergyType const Elab = (SNN - static_pow<2>(get_mass(projectileId)) -
                                static_pow<2>(get_mass(targetId))) /
                               (2 * get_mass(targetId));

    // system of initial-state
    COMBoost const boost(projectileP4NN, targetP4NN);

    auto const& originalCS = boost.getOriginalCS();
    auto const& csPrime =
        boost.getRotatedCS(); // z is along the CM motion (projectile, in Cascade)

    CORSIKA_LOGGER_DEBUG(logger_,
                         "doInteraction: interaction, projectile id={}, E={}, p3={} ",
                         projectileId, projectileP4.getTimeLikeComponent(),
                         projectileP4.getSpaceLikeComponents());
    CORSIKA_LOGGER_DEBUG(
        logger_, "doInteraction: projectile per-nucleon ENN={}, p3NN={} ",
        projectileP4NN.getTimeLikeComponent(), projectileP4NN.getSpaceLikeComponents());
    CORSIKA_LOGGER_DEBUG(
        logger_, "doInteraction: interaction, target id={}, E={}, p3={} ", targetId,
        targetP4.getTimeLikeComponent(), targetP4.getSpaceLikeComponents());
    CORSIKA_LOGGER_DEBUG(logger_, "doInteraction: target per-nucleon ENN={}, p3NN={} ",
                         targetP4NN.getTimeLikeComponent(),
                         targetP4NN.getSpaceLikeComponents());
    CORSIKA_LOGGER_DEBUG(logger_, "doInteraction: Elab={}, sqrtSNN={} ", Elab, sqrtSNN);

    int beamA = 1;
    int beamZ = 1;
    if (is_nucleus(projectileId)) {
      beamA = get_nucleus_A(projectileId);
      beamZ = get_nucleus_Z(projectileId);
      CORSIKA_LOGGER_DEBUG(logger_, "projectile: A={}, Z={} ", beamA, beamZ);
    }

    // // from corsika7 interface
    // // NEXLNK-part
    int targetA = 1;
    int targetZ = 1;
    if (is_nucleus(targetId)) {
      targetA = get_nucleus_A(targetId);
      targetZ = get_nucleus_Z(targetId);
      CORSIKA_LOGGER_DEBUG(logger_, "target: A={}, Z={} ", beamA, beamZ);
    }
    initializeEventCoM(projectileId, beamA, beamZ, targetId, targetA, targetZ, sqrtSNN);

    // create event
    int iarg = 1;
    ::epos::aepos_(iarg);
    ::epos::afinal_();

    if (epos_listing_) { // LCOV_EXCL_START
      char nam[9] = "EPOSLHC&";
      ::epos::alistf_(nam, 9);
    } // LCOV_EXCL_STOP

    // NSTORE-part

    MomentumVector P_final(originalCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    HEPEnergyType E_final = 0_GeV;

    // secondaries
    EposStack es;
    CORSIKA_LOGGER_DEBUG(logger_, "number of entries on Epos stack: {}", es.getSize());
    for (auto& psec : es) {
      if (!psec.isFinal()) continue;

      auto momentum = psec.getMomentum(csPrime);
      // transform particle output to frame defined by input 4-momenta
      auto const P4output = boost.fromCoM(FourVector{psec.getEnergy(), momentum});
      auto p3output = P4output.getSpaceLikeComponents();
      p3output.rebase(originalCS); // transform back into standard lab frame

      EposCode const eposId = psec.getPID();
      Code const pid = epos::convertFromEpos(eposId);
      HEPEnergyType const mass = get_mass(pid);
      HEPEnergyType const Ekin = sqrt(p3output.getSquaredNorm() + mass * mass) - mass;
      CORSIKA_LOGGER_TRACE(logger_,
                           " id= {}"
                           " p= {}",
                           pid, p3output.getComponents() / 1_GeV);

      auto pnew = view.addSecondary(std::make_tuple(pid, Ekin, p3output.normalized()));
      P_final += pnew.getMomentum();
      E_final += pnew.getEnergy();
    }
    CORSIKA_LOGGER_DEBUG(
        logger_,
        "conservation (all GeV): Ecm_final= n/a" /* << Ecm_final / 1_GeV*/
        ", E_final={} GeV"
        ", P_final={} GeV"
        ", no. of particles={}",
        E_final / 1_GeV, (P_final / 1_GeV).getComponents(), view.getSize());
  }
} // namespace corsika::epos
