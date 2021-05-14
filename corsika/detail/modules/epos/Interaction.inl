/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/epos/Interaction.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/modules/epos/EposStack.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <epos.hpp>

#include <string>
#include <tuple>

using namespace corsika;
using SetupParticle = setup::Stack::stack_iterator_type;

namespace corsika::epos {

  inline Interaction::Interaction(const std::string& dataPath, const bool epos_printout_on)
    : data_path_(dataPath), epos_listing_(epos_printout_on) {
    if (dataPath == "") {
      if (std::getenv("CORSIKA_DATA")) {
        data_path_ = std::string(std::getenv("CORSIKA_DATA")) + "/EPOS/";
        CORSIKA_LOG_DEBUG("Searching for EPOSLHC data tables in {}", data_path_);
      }
    }

    // initialize Eposlhc
    static bool initialized = false;
    if (!initialized) {
      initialize_eposlhc_c7();
      initialized = true;
    }
  }

  inline void Interaction::initialize_eposlhc_c7() const {

    // corsika7 ini
    int iarg = 0;
    ::epos::aaset_(iarg);
    //::epos::atitle_();

    ::epos::prnt1_.ish = 0; // debug level in epos
    ::epos::files_.ifch = 6; // output unit
    //::epos::prnt1_.iecho = 1;

    // dummy set seeds for random number generator in epos. need to fool epos checks...
    // we will use external generator
    ::epos::cseed_.seedi = 1;
    ::epos::cseed_.seedj = 1;
    ::epos::cseed_.seedc = 1;

    ::epos::enrgy_.egymin = 6.;
    ::epos::enrgy_.egymax = 2.e6;

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

    ::epos::othe1_.istore = 0;  // do not produce epos output file
    ::epos::nucl6_.infragm = 0; // keep free nucleons in fragmentation

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

    

    // dummy event
    // ::epos::hadr25_.idprojin = 1120;
    // ::epos::hadr25_.idtargin = 1120;
    // //#if __CONEX__ && __EPOS__ && __HIGHMEM__
    // //    maproj = 250
    // //#else
    // ::epos::nucl1_.maproj = 56;
    // // #endif
    // ::epos::nucl1_.laproj = 28;
    // ::epos::nucl1_.matarg = 14;
    // ::epos::nucl1_.latarg = 1;
    // ::epos::hadr1_.pnll = 200.;
    // ::epos::lept1_.engy = -1.;

    //::epos::ainit_();

    // dummy event (prepare commons)
    initialize_event_Lab(Code::Proton, Code::Argon, 100_GeV);
  }

  inline void Interaction::initialize_event_CoM(Code const idBeam, Code const idTarget,
                                                HEPEnergyType const Ecm) const {
    CORSIKA_LOG_TRACE(
        "Epos:: initialize event in CoM frame!"
        " Ecm={}",
        Ecm);
    ::epos::lept1_.engy = -1.;
    ::epos::enrgy_.ecms = -1.;
    ::epos::enrgy_.elab = -1.;
    ::epos::enrgy_.ekin = -1.;
    ::epos::hadr1_.pnll = -1.;

    ::epos::enrgy_.ecms = Ecm / 1_GeV;
    
    CORSIKA_LOG_TRACE(
        "Epos::initialize_event: inside EPOS: "
        "Ecm={}, "
        "Elab={}",
        ::epos::enrgy_.ecms, ::epos::enrgy_.elab);

    configure_particles(idBeam, idTarget);
    ::epos::ainit_();
  }

  inline void Interaction::initialize_event_Lab(Code const idBeam, Code const idTarget,
                                                HEPEnergyType const Plab) const {
    CORSIKA_LOG_TRACE(
        "Epos:: initialize event in lab. frame!"
        " Plab per nuc={} GeV",
        Plab / 1_GeV);
    ::epos::lept1_.engy = -1.;
    ::epos::enrgy_.ecms = -1.;
    ::epos::enrgy_.elab = -1.;
    ::epos::enrgy_.ekin = -1.;
    ::epos::hadr1_.pnll = -1.;
    
    // hadron-nucleon momentum
    ::epos::hadr1_.pnll = float(Plab / 1_GeV);

    CORSIKA_LOG_TRACE(
        "Epos::initialize_event: inside EPOS: "
        "Ecm={}, "
        "Elab={}, "
        "Pnll={}",
        ::epos::enrgy_.ecms, ::epos::enrgy_.elab, ::epos::hadr1_.pnll);

    configure_particles(idBeam, idTarget);
    ::epos::ainit_();
  }

  inline void Interaction::configure_particles(Code const idBeam,
                                               Code const idTarget) const {
    CORSIKA_LOG_TRACE(
        "configure_particles: setting "
        "Beam={} "
        "Target={}",
        idBeam, idTarget);
    
    //   idprojin = idtrafo("pdg","nxs",idpro)
    //   if (idpro.ne.2212) izpro = -1
    //   laproj = izpro      !proj Z
    //   maproj = iapro      !proj A
    if(is_nucleus(idBeam)){
      ::epos::hadr25_.idprojin = convertToEposRaw(Code::Proton);
      ::epos::nucl1_.laproj = get_nucleus_Z(idBeam);
      ::epos::nucl1_.maproj = get_nucleus_A(idBeam);
    } else {
      ::epos::hadr25_.idprojin = convertToEposRaw(idBeam);
      // if(idBeam!=Code::Proton)
      // 	::epos::nucl1_.laproj = -1;
      // else
      //   ::epos::nucl1_.laproj = 1;
      ::epos::nucl1_.laproj = -1;
      ::epos::nucl1_.maproj = 1;      
    }
    
    //   idtargin = idtrafo("pdg","nxs",idtar)
    //   latarg = iztar      !targ Z
    //   matarg = iatar      !targ A
    if (is_nucleus(idTarget)) {
      ::epos::hadr25_.idtargin = convertToEposRaw(Code::Proton);
      ::epos::nucl1_.matarg = get_nucleus_A(idTarget);
      ::epos::nucl1_.latarg = get_nucleus_Z(idTarget);
    } else if (idTarget == Code::Proton || idTarget == Code::Hydrogen) {
      ::epos::hadr25_.idtargin = convertToEposRaw(Code::Proton);
      ::epos::nucl1_.matarg = 1;
      ::epos::nucl1_.latarg = -1;
    } else if (idTarget == Code::Neutron) {
      ::epos::hadr25_.idtargin = convertToEposRaw(Code::Neutron);
      ::epos::nucl1_.matarg = 1;
      ::epos::nucl1_.latarg = -1;
    } else {
      throw std::runtime_error("Epos: configure_particles: target outside range!");
    }
    CORSIKA_LOG_TRACE(
        "configure_particles: inside EPOS: "
        "Id beam={}, "
        "Z beam={}, "
        "A beam={}, "
        "Id target={}, "
        "Z target={}, "
        "A target={}",
        ::epos::hadr25_.idprojin, ::epos::nucl1_.laproj, ::epos::nucl1_.maproj,
        ::epos::hadr25_.idtargin, ::epos::nucl1_.latarg, ::epos::nucl1_.matarg);
  }

  inline Interaction::~Interaction() {
    CORSIKA_LOG_DEBUG("Epos::Interaction n={} ", count_);
  }

  inline std::tuple<corsika::CrossSectionType, corsika::CrossSectionType>
  Interaction::getCrossSection(const corsika::Code BeamId, const corsika::Code TargetId,
                               const corsika::HEPEnergyType EnergyCOM) const {
    CORSIKA_LOG_DEBUG(
        "getCrossSection: input:"
        " beam={},"
        " target={},"
        " Ecm={:4.3f} GeV,",
        BeamId, TargetId, EnergyCOM / 1_GeV);

    const int iBeam = corsika::epos::getEposXSCode(
        BeamId); // 0 (can not interact, 1: proton-like, 2: pion-like, 3:kaon-like)
    if (!iBeam)
      throw std::runtime_error(
          "getCrossSection: interaction of beam hadron not defined in "
          "Epos!");

    // reset beam particle // (1: proton-like, 2: pion-like, 3:kaon-like)
    if (iBeam == 1)
      initialize_event_CoM(Code::Proton, TargetId, EnergyCOM);
    else if (iBeam == 2)
      initialize_event_CoM(Code::PiPlus, TargetId, EnergyCOM);
    else
      initialize_event_CoM(Code::KPlus, TargetId, EnergyCOM);

    //::epos::xsigma_();

    double sigProd, sigEla = 0;
    float sigTot1, sigProd1, sigEla1, sigCut1 = 0;
    if (is_nucleus(TargetId) || is_nucleus(BeamId)) {
      ::epos::crseaaepos_(sigTot1, sigProd1, sigCut1, sigEla1);
      //sigProd = ::epos::hadr5_.sigineaa;
      //sigEla = ::epos::hadr5_.sigelaaa;
      sigProd = sigProd1;
      sigEla = sigEla1;      
    } else {
      sigProd = ::epos::hadr5_.sigine;
      sigEla = ::epos::hadr5_.sigela;
    }

    // calculate cross section
    //float sigTot, sigProd, sigEla, sigCut = 0;
    //::epos::crseaaepos_(sigTot, sigProd, sigCut, sigEla);

    CORSIKA_LOG_DEBUG(
        "getCrossSection: output:"
        " sigProd={} mb,"
        " sigEla={} mb",
        sigProd, sigEla);

    return std::make_tuple(sigProd * 1_mb, sigEla * 1_mb);

    // read cross section from epos internal tables
    
    // int Abeam;
    // int iBeamId;
    // if(is_nucleus(BeamId)){
    //   Abeam = get_nucleus_A(BeamId);
    //   iBeamId = 1; //convertToEposRaw(BeamId);
    // } else {
    //   ::epos::hadr2_.idproj = convertToEposRaw(BeamId);
    //   Abeam = 1;
    // }
    // int Atarget;
    // if(is_nucleus(TargetId))
    //   Atarget = get_nucleus_A(TargetId);      
    // else
      
    // float Ekin = (EnergyLab-get_mass(BeamId)) / 1_GeV;
    // float sigProdEpos = ::epos::eposcrse_(Ekin,iBeamId);
    // float sigElaEpos = ::epos::eposelacrse_();
    // return std::make_tuple(sigProdEpos * 1_mb, sigElaEpos * 1_mb);
  }

  template <>
  inline corsika::GrammageType Interaction::getInteractionLength(
      SetupParticle const& projectile) const {

    const corsika::Code corsikaBeamId = projectile.getPID();
    const bool kInteraction = corsika::epos::canInteract(corsikaBeamId);
    CORSIKA_LOG_DEBUG(
        "Interaction: LambdaInt: \n"
        " input energy: {} GeV "
        " beam can interact: {} "
        " beam pid: {}",
        projectile.getEnergy() / 1_GeV, kInteraction, projectile.getPID());

    if (kInteraction) {
      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */

      MomentumVector const& pLab = projectile.getMomentum();
      CoordinateSystemPtr const& labCS = pLab.getCoordinateSystem();

      // assume target is at rest!!
      MomentumVector pTarget(labCS, {0_GeV, 0_GeV, 0_GeV});
      
      // total momentum and energy
      HEPEnergyType Elab = projectile.getEnergy() + constants::nucleonMass;
      MomentumVector pTotLab(labCS, {0_GeV, 0_GeV, 0_GeV});
      pTotLab += pLab;
      pTotLab += pTarget;
      auto const pTotLabNorm = pTotLab.getNorm();
      // calculate cm. energy
      const HEPEnergyType ECoM = sqrt(
          (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

      auto const* currentNode = projectile.getNode();
      const auto& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();

      si::CrossSectionType weightedProdCrossSection = mediumComposition.getWeightedSum(
          [=](corsika::Code targetID) -> si::CrossSectionType {
            return std::get<0>(this->getCrossSection(corsikaBeamId, targetID, ECoM));
          });

      CORSIKA_LOG_DEBUG(
          "Interaction: "
          "IntLength: weighted CrossSection (mb): {} ",
          weightedProdCrossSection / 1_mb);

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.getAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
      CORSIKA_LOG_DEBUG(
          "Interaction: "
          "interaction length (g/cm2): {} ",
          int_length / (0.001_kg) * 1_cm * 1_cm);

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  template <typename TSecondaryView>
  inline void Interaction::doInteraction(TSecondaryView& view) {

    auto const projectile = view.getProjectile();
    auto const corsikaBeamId = projectile.getPID();

    if (corsika::epos::canInteract(corsikaBeamId)) {

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
      if (is_nucleus(corsikaBeamId)) beamA = projectile.getNuclearA();

      HEPEnergyType const projectileMomentumLabPerNucleon = projectileMomentum / beamA;

      CORSIKA_LOG_DEBUG(
          "ProcessEPOS: "
          "DoInteraction: {} interaction ",
          corsikaBeamId);

      // define target
      // for Sibyll is always a single nucleon
      // FOR NOW: target is always at rest
      const auto eTargetLab = 0_GeV + constants::nucleonMass;
      const auto pTargetLab = MomentumVector(originalCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(eTargetLab, pTargetLab);

      CORSIKA_LOG_DEBUG(
          "Interaction: ebeam lab: {} GeV"
          "Interaction: pbeam lab: {} GeV",
          eProjectileLab / 1_GeV, pProjectileLab.getComponents());
      CORSIKA_LOG_DEBUG(
          "Interaction: etarget lab: {} GeV "
          "Interaction: ptarget lab: {} GeV",
          eTargetLab / 1_GeV, pTargetLab.getComponents() / 1_GeV);

      const FourVector PprojLab(eProjectileLab, pProjectileLab);

      // define target kinematics in lab frame
      // define boost to and from CoM frame
      // CoM frame definition in Sibyll projectile: +z
      COMBoost const boost(PprojLab, constants::nucleonMass);
      auto const& csPrime = boost.getRotatedCS();

      // just for show:
      // boost projecticle
      [[maybe_unused]] auto const PprojCoM = boost.toCoM(PprojLab);

      // boost target
      [[maybe_unused]] auto const PtargCoM = boost.toCoM(PtargLab);

      CORSIKA_LOG_DEBUG(
          "Interaction: ebeam CoM: {} GeV "
          "Interaction: pbeam CoM: {} GeV ",
          PprojCoM.getTimeLikeComponent() / 1_GeV,
          PprojCoM.getSpaceLikeComponents().getComponents(csPrime) / 1_GeV);
      CORSIKA_LOG_DEBUG(
          "Interaction: etarget CoM: {} GeV "
          "Interaction: ptarget CoM: {} GeV ",
          PtargCoM.getTimeLikeComponent() / 1_GeV,
          PtargCoM.getSpaceLikeComponents().getComponents(csPrime) / 1_GeV);

      CORSIKA_LOG_DEBUG("Interaction: position of interaction: {} ",
                        pOrig.getCoordinates());
      CORSIKA_LOG_DEBUG("Interaction: time: {} ", tOrig);

      HEPEnergyType Etot = eProjectileLab + eTargetLab;
      MomentumVector Ptot = projectile.getMomentum();
      // invariant mass, i.e. cm. energy
      HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.getSquaredNorm());

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
        [[maybe_unused]] auto const [sigProd, sigEla] =
			   getCrossSection(corsikaBeamId, targetId, Ecm);
        cross_section_of_components[i] = sigProd;
      }

      const auto targetCode =
          mediumComposition.sampleTarget(cross_section_of_components, RNG_);
      CORSIKA_LOG_DEBUG("Interaction: target selected: {} ", targetCode);

      initialize_event_Lab(corsikaBeamId, targetCode, projectileMomentumLabPerNucleon);

      // // from corsika7 interface
      // // NEXLNK-part

      // // projectile
      // ::epos::hadr25_.idprojin =
      //     convertToEposRaw(corsikaBeamId); // 1120 ; // id "NEXUS code"
      // ::epos::nucl1_.laproj = -1;          // Z (-1 for hadron)
      // ::epos::nucl1_.maproj = 1;           // A

      // // target
      // int targetMassNumber = 1;     // proton
      // if (is_nucleus(targetCode)) { // nucleus
      //   targetMassNumber = get_nucleus_A(targetCode);
      //   if (targetMassNumber > maxTargetMassNumber_)
      //     throw std::runtime_error("Epos target mass outside range.");
      // 	::epos::nucl1_.matarg = targetMassNumber;
      // 	::epos::nucl1_.latarg = get_nucleus_Z(targetCode);
      // } else {
      //   if (targetCode != Proton::code && targetCode != Neutron::code)
      //     throw std::runtime_error("Epos target not possible.");
      //   // proton or neutron target
      //   ::epos::hadr25_.idtargin = convertToEposRaw(targetCode);
      //   if (targetCode == Proton::code)
      //     ::epos::nucl1_.latarg = 1; // Z
      //   else
      //     ::epos::nucl1_.latarg = -1; // Z (-1 with id 1220 for neutron)
      //   ::epos::nucl1_.matarg = 1;    // A
      // }
      // CORSIKA_LOG_DEBUG("Interaction: target epos code/A: {}", targetMassNumber);

      // // hadron-nucleon momentum
      // ::epos::hadr1_.pnll = float(projectileMomentumLabPerNucleon / 1_GeV); // float(200);

      // // C  SET ENGY NEGATIVE TO FORCE CALCULATION IN LAB FRAME
      // ::epos::lept1_.engy = -1.;
      // ::epos::enrgy_.ecms = -1.;
      // ::epos::enrgy_.elab = -1.;
      // ::epos::enrgy_.ekin = -1.;

      // // C  INTIALIZE ENERGY AND PARTICLE DEPENDENT PORTION OF EPOS/NEXUS
      // // C  AT THE FIRST CALL: READ ALSO DATA SETS
      // ::epos::ainit_();

      // create event
      int iarg = 1;
      ::epos::aepos_(iarg);

      ::epos::afinal_();

      if (epos_listing_) ::epos::alistf_("EPOSLHC&");

      // NSTORE-part

      MomentumVector Plab_final(originalCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      HEPEnergyType Elab_final = 0_GeV;

      // secondaries
      EposStack es;
      CORSIKA_LOG_DEBUG("EPOSLHC number of particles: {}", es.getSize());
      // int i=-1;
      for (auto& psec : es) {
        //++i;
        if (!psec.isFinal()) continue;

        // CORSIKA_LOG_DEBUG("EPOS: i, id, energy, mass, type, state: {} {} {} {} {}", i,
        //                   ::epos::cptl_.idptl[i], ::epos::cptl_.pptl[3][i],
        //                   ::epos::cptl_.pptl[4][i], ::epos::cptl_.ityptl[i],
        //                   ::epos::cptl_.istptl[i]);
        // CORSIKA_LOG_DEBUG("id, energy: {} {}", psec.getPID(), psec.getEnergy());
        // int id = abs(static_cast<int>(psec.getPID()));
        // CORSIKA_LOG_DEBUG("epos id to pdg: {}",
        //                   ::epos::idtrafo_("nxs", "pdg", id));

        auto momentum = psec.getMomentum(zAxisFrame);
        auto const energy = psec.getEnergy();

        momentum.rebase(originalCS); // transform back into standard lab frame

        auto const pid = corsika::epos::convertFromEpos(psec.getPID());
        CORSIKA_LOG_DEBUG(
            " id= {}"
            " p= {}",
            pid, momentum.getComponents() / 1_GeV);
        auto pnew =
            view.addSecondary(std::make_tuple(pid, energy, momentum, pOrig, tOrig));
        Plab_final += pnew.getMomentum();
        Elab_final += pnew.getEnergy();
      }
      CORSIKA_LOG_DEBUG(
          "conservation (all GeV): Ecm_final= n/a" /* << Ecm_final / 1_GeV*/
          ", Elab_final={}"
          ", Plab_final={}",
          Elab_final / 1_GeV, (Plab_final / 1_GeV).getComponents());
    }

    /*
      maproj=maprojxs
      laproj=laprojxs
      matarg=matargxs
      latarg=latargxs
      idproj=idprojxs
      idtarg=idtargxs
      amproj=xsamproj
      amtarg=xsamtarg
      call idspin(idproj,ispin,jspin,istra)
      isoproj=sign(1,idproj)*ispin
      call idspin(idtarg,ispin,jspin,istra)
      isotarg=sign(1,idtarg)*ispin

      engy=sngl(xsengy)
      elab=sngl(xselab)
      ecms=sngl(xsecms)
      ekin=sngl(xsekin)
      pnll=sngl(xspnll)
      pnullx=sngl(xspnullx)
      yhaha=sngl(xsyhaha)
      ypjtl=sngl(xsypjtl)
      detap=xsdetap
      detat=xsdetat
      tpro=xstpro
      zpro=xszpro
      ttar=xsttar
      ztar=xsztar

c      xsbminim=dble(bminim)     !not needed and can interfer with other MC
c      xsbmaxim=dble(bmaxim)

      call iclass(idproj,iclpro)
      call iclass(idtarg,icltar)
      call emsini(engy,idproj,idtarg)
      call paramini(1)
      bkmxndif=conbmxndif()
      bkmx=conbmx()
      xsbkmx=dble(bkmx)

      if(maproj.gt.1.or.matarg.gt.1)then
        xsbmax=xsrmproj+xsrmtarg
      else
        xsbmax=xsbkmx
      endif

      bimevt=-1
      bmax=sngl(xsbmax)
      rmproj=sngl(xsrmproj)
      rmtarg=sngl(xsrmtarg)
      rcproj=xsrcproj
      rctarg=xsrctarg
      call xsigma                          !set some variabkle according to xs
      if(idtarg.eq.0)idtarg=1120           !air = nucleus

     */
  }
} // namespace corsika::epos
