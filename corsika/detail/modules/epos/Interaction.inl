/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/epos/Interaction.hpp>
#include <corsika/modules/epos/EposStack.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/utility/CorsikaData.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <epos.hpp>

#include <string>
#include <tuple>

using namespace corsika;
using SetupParticle = setup::Stack::stack_iterator_type;

namespace corsika::epos {

  inline Interaction::Interaction(std::string const& dataPath,
                                  bool const epos_printout_on)
      : data_path_(dataPath)
      , epos_listing_(epos_printout_on) {
    if (dataPath == "") {
      data_path_ = (std::string(corsika_data("EPOS").c_str()) + "/").c_str();
    }
    // initialize Eposlhc
    static bool initialized = false;
    if (!initialized) {
      initialize();
      initialized = true;
    }
    setParticlesStable();
  }

  inline void Interaction::setParticlesStable() const {
    CORSIKA_LOGGER_DEBUG(logger_,
                         "set all particles known to CORSIKA stable inside EPOS..");
    for (auto& p : get_all_particles()) {
      if (!is_hadron(p)) continue;
      int const eid = convertToEposRaw(p);
      if (eid != 0) {
        ::epos::nodcy_.nrnody = ::epos::nodcy_.nrnody + 1;
        ::epos::nodcy_.nody[::epos::nodcy_.nrnody - 1] = eid;
      }
    }
  }

  inline bool Interaction::isValidTarget(Code const TargetId) const {
    return is_nucleus(TargetId) && (get_nucleus_A(TargetId) < maxTargetMassNumber_);
  }

  inline void Interaction::initialize() const {

    CORSIKA_LOGGER_DEBUG(logger_, "initializing...");

    // corsika7 ini
    int iarg = 0;
    ::epos::aaset_(iarg);

    // debug output settings
    ::epos::prnt1_.ish = 0;    // debug level in epos, 0: off, 6: medium output
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

    ::epos::othe2_.iframe = 12; // lab frame, target at rest

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

    // dummy event (prepare commons)
    initializeEventLab(Code::Iron, Iron::nucleus_A, Iron::nucleus_Z, Code::Argon,
                       Argon::nucleus_A, Argon::nucleus_Z, 100_GeV);
  }

  inline void Interaction::initializeEventCoM(Code const idBeam, int const iBeamA,
                                              int const iBeamZ, Code const idTarget,
                                              int const iTargetA, int const iTargetZ,
                                              HEPEnergyType const Ecm) const {
    CORSIKA_LOGGER_TRACE(logger_,
                         "initialize event in CoM frame!"
                         " Ecm={}",
                         Ecm);
    ::epos::lept1_.engy = -1.;
    ::epos::enrgy_.ecms = -1.;
    ::epos::enrgy_.elab = -1.;
    ::epos::enrgy_.ekin = -1.;
    ::epos::hadr1_.pnll = -1.;

    ::epos::enrgy_.ecms = Ecm / 1_GeV; // -> c.m.s. frame

    CORSIKA_LOGGER_TRACE(logger_,
                         "inside EPOS: "
                         "Ecm={}, "
                         "Elab={}",
                         ::epos::enrgy_.ecms, ::epos::enrgy_.elab);

    configureParticles(idBeam, iBeamA, iBeamZ, idTarget, iTargetA, iTargetZ);
    ::epos::ainit_();
  }

  inline void Interaction::initializeEventLab(Code const idBeam, int const iBeamA,
                                              int const iBeamZ, Code const idTarget,
                                              int const iTargetA, int const iTargetZ,
                                              HEPEnergyType const Plab) const {
    CORSIKA_LOGGER_TRACE(logger_,
                         "initialize event in lab. frame!"
                         " Plab per nuc={} GeV",
                         Plab / 1_GeV);
    ::epos::lept1_.engy = -1.;
    ::epos::enrgy_.ecms = -1.;
    ::epos::enrgy_.elab = -1.;
    ::epos::enrgy_.ekin = -1.;
    ::epos::hadr1_.pnll = -1.;

    // hadron-nucleon momentum
    ::epos::hadr1_.pnll = float(Plab / 1_GeV); // -> lab frame

    CORSIKA_LOGGER_TRACE(logger_,
                         "inside EPOS: "
                         "Ecm={}, "
                         "Elab={}, "
                         "Pnll={}",
                         ::epos::enrgy_.ecms, ::epos::enrgy_.elab, ::epos::hadr1_.pnll);

    configureParticles(idBeam, iBeamA, iBeamZ, idTarget, iTargetA, iTargetZ);
    ::epos::ainit_();
  }

  inline void Interaction::configureParticles(Code const idBeam, int const iBeamA,
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
    } else {
      throw std::runtime_error("Epos: target outside range!");
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

  inline Interaction::~Interaction() { CORSIKA_LOGGER_DEBUG(logger_, "n={} ", count_); }

  inline std::tuple<CrossSectionType, CrossSectionType> Interaction::calcCrossSectionCoM(
      Code const BeamId, int const BeamA, int const BeamZ, Code const TargetId,
      int const TargetA, int const TargetZ, const HEPEnergyType EnergyCOM) const {
    CORSIKA_LOGGER_DEBUG(logger_,
                         "calcCrossSection: input:"
                         " beamId={}, beamA={}, beamZ={}"
                         " target={}, targetA={}, targetZ={}"
                         " Ecm={:4.3f} GeV,",
                         BeamId, BeamA, BeamZ, TargetId, TargetA, TargetZ,
                         EnergyCOM / 1_GeV);

    const int iBeam = corsika::epos::getEposXSCode(
        BeamId); // 0 (can not interact, 1: proton-like, 2: pion-like, 3:kaon-like)
    if (!iBeam)
      throw std::runtime_error(
          "calcCrossSectionCoM: interaction of beam hadron not defined in "
          "Epos!");

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
    else
      throw std::runtime_error(
          "calcCrossSectionCoM: interaction of beam hadron not defined in "
          "Epos!");

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

  inline std::tuple<corsika::CrossSectionType, corsika::CrossSectionType>
  Interaction::readCrossSectionTableLab(Code const BeamId, int const BeamA,
                                        int const BeamZ, Code const TargetId,
                                        HEPEnergyType const EnergyLab) const {
    CORSIKA_LOGGER_DEBUG(logger_,
                         "readCrossSectionTableLab: input: "
                         "beamId={}, "
                         "beamA={}, "
                         "beamZ={} "
                         "targetId={}, "
                         "ELab={:4.3f} GeV,",
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
      int const iBeam = corsika::epos::getEposXSCode(
          BeamId); // 0 (can not interact, 1: pion-like, 2: proton-like, 3:kaon-like)
      CORSIKA_LOGGER_TRACE(logger_,
                           "projectile cross section type={} "
                           "(0: cannot interact, 1:pion, 2:baryon, 3:kaon)",
                           iBeam);

      ::epos::had10_.iclpro = iBeam;
      Abeam = 1;
      Ekin = (EnergyLab - get_mass(BeamId)) / 1_GeV;
    }
    if (Ekin < 0) {
      CORSIKA_LOGGER_ERROR(logger_,
                           "Negative kinetic energy!"
                           "Ekin={}",
                           Ekin);
      throw std::runtime_error("Epos cross section failed! Negative kinetic energy!");
    }

    int Atarget = 1;
    if (is_nucleus(TargetId)) { Atarget = get_nucleus_A(TargetId); }

    int iMode = 3; // 0: air, >0 not air

    CORSIKA_LOGGER_DEBUG(logger_,
                         "inside Epos "
                         "beamId={}, beamXS={}",
                         ::epos::hadr2_.idproj, ::epos::had10_.iclpro);

    // cross section from table, FAST
    float sigProdEpos = ::epos::eposcrse_(Ekin, Abeam, Atarget, iMode);
    // sig-el from analytic calculation, no fast
    float sigElaEpos = ::epos::eposelacrse_(Ekin, Abeam, Atarget, iMode);

    return std::make_tuple(sigProdEpos * 1_mb, sigElaEpos * 1_mb);
  }

  inline std::tuple<corsika::CrossSectionType, corsika::CrossSectionType>
  Interaction::getCrossSectionLab(corsika::Code const BeamId, int const BeamA,
                                  int const BeamZ, corsika::Code const TargetId,
                                  int const TargetA, int const TargetZ,
                                  const corsika::HEPEnergyType EnergyLab) const {
    CORSIKA_LOGGER_DEBUG(logger_,
                         "getCrossSectionLab: input:"
                         " beamId={}, beamA={}, beamZ={}"
                         " target={}, targetA={}, targetZ={}"
                         " ELab={:4.3f} GeV,",
                         BeamId, BeamA, BeamZ, TargetId, TargetA, TargetZ,
                         EnergyLab / 1_GeV);
    return readCrossSectionTableLab(BeamId, BeamA, BeamZ, TargetId, EnergyLab);
  }

  template <>
  inline corsika::GrammageType Interaction::getInteractionLength(
      SetupParticle const& projectile) const {

    const corsika::Code corsikaBeamId = projectile.getPID();
    const bool kInteraction = corsika::epos::canInteract(corsikaBeamId);
    CORSIKA_LOGGER_DEBUG(logger_,
                         "InteractionLength: input: \n"
                         " energy: {} GeV "
                         " beam can interact: {} "
                         " beam pid: {}",
                         projectile.getEnergy() / 1_GeV, kInteraction,
                         projectile.getPID());

    if (kInteraction) {

      // define projectile nuclei
      int beamA = 1;
      int beamZ = 1;
      if (is_nucleus(corsikaBeamId)) {
        beamA = get_nucleus_A(corsikaBeamId);
        beamZ = get_nucleus_Z(corsikaBeamId);
      }

      // get target from environment
      MomentumVector const& pLab = projectile.getMomentum();
      CoordinateSystemPtr const& labCS = pLab.getCoordinateSystem();

      // assume target is at rest!!
      MomentumVector pTarget(labCS, {0_GeV, 0_GeV, 0_GeV});

      // total momentum and energy
      HEPEnergyType Elab = projectile.getEnergy() + constants::nucleonMass;

      auto const* currentNode = projectile.getNode();
      const auto& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();

      si::CrossSectionType weightedProdCrossSection = mediumComposition.getWeightedSum(
          [=](corsika::Code targetID) -> si::CrossSectionType {
            return std::get<0>(this->getCrossSectionLab(corsikaBeamId, beamA, beamZ,
                                                        targetID, get_nucleus_A(targetID),
                                                        get_nucleus_Z(targetID), Elab));
          });

      CORSIKA_LOGGER_DEBUG(logger_, "InteractionLength: weighted CrossSection (mb): {} ",
                           weightedProdCrossSection / 1_mb);

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.getAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
      CORSIKA_LOGGER_DEBUG(logger_, "interaction length (g/cm2): {} ",
                           int_length / (0.001_kg) * 1_cm * 1_cm);

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  template <typename TSecondaryView>
  inline void Interaction::doInteraction(TSecondaryView& view) {

    auto const projectile = view.getProjectile();
    auto const corsikaBeamId = projectile.getPID();

    CORSIKA_LOGGER_DEBUG(logger_, "doInteraction: {} interaction, Elab={} ",
                         corsikaBeamId, projectile.getEnergy());

    if (corsika::epos::canInteract(corsikaBeamId)) {
      count_ = count_ + 1;
      // position and time of interaction, not used in Epos
      Point const pOrig = projectile.getPosition();
      TimeType const tOrig = projectile.getTime();

      // define projectile
      HEPEnergyType const eProjectileLab = projectile.getEnergy();
      auto const pProjectileLab = projectile.getMomentum();
      auto const projectileMomentum = pProjectileLab.getNorm();
      CoordinateSystemPtr const& originalCS = pProjectileLab.getCoordinateSystem();

      // epos frame with z along the projectile direction
      CoordinateSystemPtr const zAxisFrame = make_rotationToZ(originalCS, pProjectileLab);

      int beamA = 1;
      int beamZ = 1;
      if (is_nucleus(corsikaBeamId)) {
        beamA = get_nucleus_A(corsikaBeamId);
        beamZ = get_nucleus_Z(corsikaBeamId);
        CORSIKA_LOGGER_DEBUG(logger_, "A={}, Z={} ", beamA, beamZ);
      }

      HEPEnergyType const projectileMomentumLabPerNucleon = projectileMomentum / beamA;

      // define target

      // sample target mass number
      auto const* currentNode = projectile.getNode();
      auto const& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();
      // get cross sections for target materials
      /*
        Here we read the cross section from the interaction model again,
        should be passed from getInteractionLength if possible
       */
      //#warning reading interaction cross section again, should not be necessary
      auto const& compVec = mediumComposition.getComponents();
      std::vector<CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        [[maybe_unused]] auto const [sigProd, sigEla] = getCrossSectionLab(
            corsikaBeamId, beamA, beamZ, targetId, get_nucleus_A(targetId),
            get_nucleus_Z(targetId), eProjectileLab);
        cross_section_of_components[i] = sigProd;
      }

      const auto targetCode =
          mediumComposition.sampleTarget(cross_section_of_components, RNG_);
      CORSIKA_LOGGER_DEBUG(logger_, "target selected: {} ", targetCode);

      // // from corsika7 interface
      // // NEXLNK-part
      int targetA = 1;
      int targetZ = 1;
      if (is_nucleus(targetCode)) {
        targetA = get_nucleus_A(targetCode);
        targetZ = get_nucleus_Z(targetCode);
      }
      initializeEventLab(corsikaBeamId, beamA, beamZ, targetCode, targetA, targetZ,
                         projectileMomentumLabPerNucleon);

      // create event
      int iarg = 1;
      ::epos::aepos_(iarg);

      ::epos::afinal_();

      if (epos_listing_) {
        char nam[9] = "EPOSLHC&";
        ::epos::alistf_(nam, 9);
      }

      // NSTORE-part

      MomentumVector Plab_final(originalCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      HEPEnergyType Elab_final = 0_GeV;

      // secondaries
      EposStack es;
      CORSIKA_LOGGER_DEBUG(logger_, "number of particles: {}", es.getSize());
      for (auto& psec : es) {
        if (!psec.isFinal()) continue;

        auto momentum = psec.getMomentum(zAxisFrame);

        momentum.rebase(originalCS); // transform back into standard lab frame

        EposCode const eposId = psec.getPID();
        Code const pid = corsika::epos::convertFromEpos(eposId);
        CORSIKA_LOGGER_TRACE(logger_,
                             " id= {}"
                             " p= {}",
                             pid, momentum.getComponents() / 1_GeV);
        if (!is_nucleus(pid)) {
          auto pnew = view.addSecondary(std::make_tuple(pid, momentum, pOrig, tOrig));
          Plab_final += pnew.getMomentum();
          Elab_final += pnew.getEnergy();
        } else {
          unsigned int A = 0;
          unsigned int Z = 0;
          if (pid == Code::Deuterium) {
            A = 2;
            Z = 1;
          } else if (pid == Code::Tritium) {
            A = 3;
            Z = 1;
          } else if (pid == Code::Helium) {
            A = 4;
            Z = 2;
          } else {
            Z = get_nucleus_Z(eposId);
            A = get_nucleus_Z(eposId);
          }
          auto pnew = view.addSecondary(
              std::make_tuple(get_nucleus_code(A, Z), momentum, pOrig, tOrig));
          Plab_final += pnew.getMomentum();
          Elab_final += pnew.getEnergy();
        }
      }
      CORSIKA_LOGGER_DEBUG(
          logger_,
          "conservation (all GeV): Ecm_final= n/a" /* << Ecm_final / 1_GeV*/
          ", Elab_final={}"
          ", Plab_final={}",
          Elab_final / 1_GeV, (Plab_final / 1_GeV).getComponents());
    } else
      CORSIKA_LOGGER_WARN(
          logger_, "Projectile not configured for interaction! This is likely an error!");
  }
} // namespace corsika::epos
