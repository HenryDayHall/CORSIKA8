/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <tuple>
#include <stdexcept>

#include <boost/filesystem/path.hpp>
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

 inline Interaction::Interaction(boost::filesystem::path const& mpiInitFile,
                                  bool const print_listing)
      : print_listing_(print_listing)
      , pythiaMain_{CORSIKA_Pythia8_XML_DIR, false}
      , pythiaColl_{CORSIKA_Pythia8_XML_DIR, false} {
    Pythia8::RndmEngine* rndm = new corsika::pythia8::Random();
    pythiaColl_.setRndmEnginePtr(rndm);
    pythiaMain_.setRndmEnginePtr(rndm);

      CORSIKA_LOG_INFO("Pythia8 MPI init file: {}", mpiInitFile.native());
      // Main Pythia object for managing the cascade evolution.
      // Can also do decays, but no hard processes.
      
      pythiaMain_.readString("ProcessLevel:all = off");
      pythiaMain_.readString("211:mayDecay = on"); // TODO: probably not necessary, but ask Torjbörn maybe
      pythiaMain_.readString("13:mayDecay  = on");
      pythiaMain_.readString("321:mayDecay = on");
      pythiaMain_.readString("130:mayDecay = on");

      // Reduce statistics printout to relevant ones.
      pythiaMain_.readString("Stat:showProcessLevel = off");  
      pythiaMain_.readString("Stat:showPartonLevel = off");  
      
      // Add Argon, since not in Particle data. id:all = name antiName
      // spinType chargeType colType m0 mWidth mMin mMax tau0. 
      pythiaMain_.readString("1000180400:all = 40Ar 40Arbar 1 54 0 "
        "37.22474 0. 0. 0. 0.");
        
      if (!pythiaMain_.init())
          throw std::runtime_error("Pythia::Interaction: Initialization failed!");
      

    // TODO: do we need this?
    //~ CORSIKA_LOG_INFO("Configuring Pythia8 from: {}", CORSIKA_Pythia8_XML_DIR);
    
     // Secondary Pythia object for performing individual collisions.
    // Variable incoming beam type and energy.
      pythiaColl_.readString("Beams:allowVariableEnergy = on");
      pythiaColl_.readString("Beams:allowIDAswitch = on");
      
      // Set up for fixed-target collisions.
      pythiaColl_.readString("Beams:frameType = 3"); // arbitrary frame, need to define full 4-momenta
      pythiaColl_.settings.parm("Beams:pzA", eMaxLab_ / 1_GeV);
      pythiaColl_.readString("Beams:pzB = 0.");
      
      // Must use the soft and low-energy QCD processes.
      pythiaColl_.readString("SoftQCD:all = on");
      pythiaColl_.readString("LowEnergyQCD:all = on");
      
      // Decays to be done by pythiaMain_.
      pythiaColl_.readString("HadronLevel:Decay = off");
      
      // Reduce printout and relax energy-momentum conservation.
      pythiaColl_.readString("Print:quiet = on");
      pythiaColl_.readString("Check:epTolErr = 0.1");
      pythiaColl_.readString("Check:epTolWarn = 0.0001");
      pythiaColl_.readString("Check:mTolErr = 0.01");

      // Redure statistics printout to relevant ones.
      pythiaColl_.readString("Stat:showProcessLevel = off");  
      pythiaColl_.readString("Stat:showPartonLevel = off");  
      
      
      bool const reuse = true; // TODO: make this more flexible
      // Reuse MPI initialization file if it exists; else create a new one.
      if (reuse) pythiaColl_.readString("MultipartonInteractions:reuseInit = 3");
      else       pythiaColl_.readString("MultipartonInteractions:reuseInit = 1");
      pythiaColl_.settings.word("MultipartonInteractions:initFile", mpiInitFile.native());
      
      // initialize
      // we can't test this block, LCOV_EXCL_START
      if (!pythiaColl_.init())
          throw std::runtime_error("Pythia::Interaction: Initialization failed!");
      // LCOV_EXCL_STOP


    //~ // any decays in pythia? if yes need to define which particles
    //~ if (internalDecays_) {
      //~ // define which particles are passed to corsika, i.e. which particles make it into
      //~ // history even very shortlived particles like charm or pi0 are of interest here
      //~ std::vector<Code> const HadronsWeWantTrackedByCorsika = {
          //~ Code::PiPlus, Code::PiMinus, Code::Pi0,        Code::KMinus,     Code::KPlus,
          //~ Code::K0Long, Code::K0Short, Code::SigmaPlus,  Code::SigmaMinus, Code::Lambda0,
          //~ Code::Xi0,    Code::XiMinus, Code::OmegaMinus, Code::DPlus,      Code::DMinus,
          //~ Code::D0,     Code::D0Bar};

      //~ Interaction::setStable(HadronsWeWantTrackedByCorsika);
    //~ }
  }

  //~ inline void Interaction::setStable(std::vector<Code> const& particleList) {
    //~ for (auto p : particleList) Interaction::setStable(p);
  //~ }

  //~ inline void Interaction::setUnstable(Code const pCode) {
    //~ CORSIKA_LOG_DEBUG("Pythia::Interaction: setting {} unstable..", pCode);
    //~ Pythia8::Pythia::particleData.mayDecay(static_cast<int>(get_PDG(pCode)), true);
  //~ }

  //~ inline void Interaction::setStable(Code const pCode) {
    //~ CORSIKA_LOG_DEBUG("Pythia::Interaction: setting {} stable..", pCode);
    //~ Pythia8::Pythia::particleData.mayDecay(static_cast<int>(get_PDG(pCode)), false);
  //~ }

  inline bool Interaction::isValid(Code const projectileId, Code const targetId,
                                   HEPEnergyType const sqrtS) const {
    if (is_nucleus(projectileId)) // not yet possible with Pythia
        return false;
        
    // TODO: check sqrtS for validity
    
    return std::find(validTargets_.begin(), validTargets_.end(), targetId) != validTargets_.end();
  }

  inline bool Interaction::canInteract(Code const pCode) const {
    return true; // TODO: implement this
  }

  inline std::tuple<CrossSectionType, CrossSectionType>
  Interaction::getCrossSectionInelEla(Code const projectileId, Code const targetId,
                                      FourMomentum const& projectileP4,
                                      FourMomentum const& targetP4) const {
    HEPEnergyType const CoMenergy =
        is_nucleus(targetId)
            ? (projectileP4 + targetP4 / get_nucleus_A(targetId)).getNorm()
            : (projectileP4 + targetP4).getNorm();

    if (is_nucleus(targetId) || is_nucleus(projectileId)) {
      CORSIKA_LOG_ERROR(
          "Pythia8::Interaction::getCrossSectionInelEla() called with nuclear projectile "
          "or target");
      return std::make_tuple(CrossSectionType::zero(), CrossSectionType::zero());
    }

    if (!isValid(projectileId, targetId, CoMenergy)) {
      return std::make_tuple(CrossSectionType::zero(), CrossSectionType::zero());
    }

    // input particle PDG
    auto const pdgCodeBeam = static_cast<int>(get_PDG(projectileId));
    auto const pdgCodeTarget = static_cast<int>(get_PDG(targetId));
    double const ecm_GeV = CoMenergy * (1 / 1_GeV);
    
    auto const sigTot = pythiaColl_.getSigmaTotal(pdgCodeBeam, 2212, ecm_GeV) * 1_mb;
    auto const sigEla = pythiaColl_.getSigmaPartial(pdgCodeBeam, 2212, ecm_GeV, 2) * 1_mb;

    return std::make_tuple(sigTot - sigEla, sigEla);
  }

  CrossSectionType Interaction::getCrossSection(Code const projectileId,
                                                Code const targetId,
                                                FourMomentum const& projectileP4,
                                                FourMomentum const& targetP4) const {

    if (!is_nucleus(targetId) || get_nucleus_A(targetId) == 1) {
      auto const [sigProd, sigEla] =
          getCrossSectionInelEla(projectileId, targetId, projectileP4, targetP4);
      return sigProd + sigEla;
    } else {
      auto const [sigProd, sigEla] =
          getCrossSectionInelEla(projectileId, Code::Proton, projectileP4, targetP4);
      auto const sigTot = sigProd + sigEla;
      return sigTot * get_nucleus_A(targetId) / getAverageSubcollisions(targetId, sigTot);
    }
  }

  double Interaction::getAverageSubcollisions(Code targetId,
                                              CrossSectionType sigTot) const {
    if (targetId == Code::Proton || targetId == Code::Neutron ||
        targetId == Code::AntiProton || targetId == Code::AntiNeutron)
      return 1;

    auto const Z = get_nucleus_Z(targetId);
    auto const A = get_nucleus_A(targetId);

    double nCollAvg;
    
    if (Z == 7 && A == 14) nCollAvg = (sigTot < 31._mb) // this is from the paper
       ? 1. + 0.017/1_mb * sigTot : 1.2 + 0.0105/1_mb * sigTot;
    else if (Z == 8 && A == 16) nCollAvg = (sigTot < 16._mb) // provided by Torbjörn Sjöstrand
       ? 1. + 0.0245/1_mb * sigTot : 1.2 + 0.012/1_mb * sigTot; 
    else if (Z == 18 && A == 40) nCollAvg = (sigTot < 10._mb)
       ? 1. + 0.050/1_mb * sigTot : 1.28 + 0.022/1_mb * sigTot;
    else {
        CORSIKA_LOG_ERROR(fmt::format("Pythia8 cross-sections not defined for ({},{}) nucleus", A, Z));
        nCollAvg = 0;
    }
    
    return nCollAvg;
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
    
    int const idNow = static_cast<int>(get_PDG(projectileId));
    
    // References to the two event records. Clear main event record.
    Pythia8::Event& eventMain = pythiaMain_.event;
    Pythia8::Event& eventColl = pythiaColl_.event;
    
    COMBoost const labFrameBoost{targetP4.getSpaceLikeComponents(), get_mass(targetId)};
    auto const proj4MomLab = labFrameBoost.toCoM(projectileP4);
    auto const& rotCS = labFrameBoost.getRotatedCS();
    auto const pProjLab = proj4MomLab.getSpaceLikeComponents().getComponents(rotCS);
    
    auto constexpr invGeV = 1 / 1_GeV;
    
    Pythia8::Vec4 const pNow{pProjLab.getX() * invGeV, pProjLab.getY() * invGeV, pProjLab.getZ() * invGeV, proj4MomLab.getTimeLikeComponent() * invGeV};
    
    // Insert incoming particle in cleared main event record.
    int unsuccessful_iterations{0};
    do { // retry event generation if Pythia gets stuck
      eventMain.clear();
      eventMain.append(90, -11, 0, 0, 1, 1, 0, 0, pNow, pNow.mCalc());
      int const iHad = eventMain.append(idNow, 12, 0, 0, 0, 0, 0, 0, pNow,
                                        get_mass(projectileId) * (1 / 1_GeV));

      Pythia8::Vec4 const
          vNow{}; // production vertex; useless but necessary (?) TODO: ask TS

      eventMain[iHad].vProd(vNow);

      auto const [Anow, Znow] = std::invoke([targetId]() {
        if (targetId == Code::Proton) {
          return std::make_pair(1, 1);
        } else if (targetId == Code::Neutron) {
          return std::make_pair(1, 0);
        } else if (is_nucleus(targetId)) {
          return std::make_pair<int, int>(get_nucleus_A(targetId),
                                          get_nucleus_Z(targetId));
        } else {
          return std::make_pair(0, 0); // TODO: what to do in this case?
        }
      });

      // Set up for collisions on a nucleus.
      int np = Znow;
      int nn = Anow - Znow;
      int sizeOld = 0;
      int sizeNew = 0;
      Pythia8::Vec4 const dirNow = pNow / pNow.pAbs();
      Pythia8::Rndm& rndm = pythiaMain_.rndm;

      double constexpr mp = get_mass(Code::Proton) / 1_GeV;
      double const sqrtSNN_GeV = (pNow + Pythia8::Vec4{0, 0, 0, mp}).mCalc();

      auto const nCollAvg = getAverageSubcollisions(
          targetId, pythiaColl_.getSigmaTotal(idNow, 2212, sqrtSNN_GeV) * 1_mb);
      double const probMore = 1. - 1. / nCollAvg;

      // Loop over varying number of hit nucleons in target nucleus.
      for (int iColl = 1; iColl <= Anow; ++iColl) {
        if (iColl > 1 && rndm.flat() > probMore) break;

        // Pick incoming projectile: trivial for first subcollision, else ...
        int iProj    = iHad;
        int procType = 0;

        // ... find highest-pLongitudinal particle from latest subcollision.
        if (iColl > 1) {
          iProj = 0;
          double pMax = 0.;
          for (int i = sizeOld; i < sizeNew; ++i)
          if ( eventMain[i].isFinal() && eventMain[i].isHadron()) {
            if (double const pp = Pythia8::dot3(dirNow, eventMain[i].p()); pp > pMax) {
              iProj = i;
              pMax  = pp;
            }
          }

          // No further subcollision if no particle with enough energy.
          if ( iProj == 0 || eventMain[iProj].e() - eventMain[iProj].m()
            < eKinMinLab_/1_GeV) break;

          // Choose process; only SD or ND at perturbative energies.
          double const eCMSub = (eventMain[iProj].p() + Pythia8::Vec4{0, 0, 0, mp}).mCalc();
          if (eCMSub > 10.) procType = (rndm.flat() < probSD_) ? 4 : 1;
        }

        // Pick one p or n from target.
        int const idProj = eventMain[iProj].id();
        bool const doProton = rndm.flat() < (np / double(np + nn));
        if (doProton) np--;
        else          nn--;
        int const idNuc = doProton ? 2212 : 2112;

        // Perform the projectile-nucleon subcollision.
        pythiaColl_.setBeamIDs(idProj, idNuc);
        pythiaColl_.setKinematics(eventMain[iProj].p(), Pythia8::Vec4());
        
        if (!pythiaColl_.next(procType)) {
          CORSIKA_LOG_WARN("Pythia collision next() failed {} {} {}",
                           eventMain[iProj].p(), idProj, idNuc);
          eventMain.clear();
          ++unsuccessful_iterations;
          goto event_repeat; // retry event, last remaining good use-cse for goto
          
          throw std::runtime_error("Pythia collision next() failed!");
        }

        // Insert target nucleon. Mothers are (0,iProj) to mark who it
        // interacted with. Always use proton mass for simplicity.
        int const statusNuc = (iColl == 1) ? -181 : -182;
        int const iNuc = eventMain.append( idNuc, statusNuc, 0, iProj, 0, 0, 0, 0,
          0., 0., 0., mp, mp);
        eventMain[iNuc].vProdAdd(vNow);
        
        // Update full energy of the event with the proton mass.
        eventMain[0].e( eventMain[0].e() + mp);
        eventMain[0].m( eventMain[0].p().mCalc() );

        // Insert secondary produced particles (but skip intermediate partons)
        // into main event record and shift to correct production vertex.
        sizeOld = eventMain.size();
        for (int iSub = 3; iSub < eventColl.size(); ++iSub) {
          if (!eventColl[iSub].isFinal()) continue;
          int const iNew = eventMain.append(eventColl[iSub]);
          eventMain[iNew].mothers(iNuc, iProj);
          eventMain[iNew].vProdAdd(vNow);
        }
        sizeNew = eventMain.size();

        // Update daughters of colliding hadrons and other history.
        eventMain[iProj].daughters(sizeOld, sizeNew - 1);
        eventMain[iNuc].daughters(sizeOld, sizeNew - 1);
        eventMain[iProj].statusNeg();
        double dTau = (iColl == 1) ? (vNow.e() - eventMain[iHad].tProd())
          * eventMain[iHad].m() / eventMain[iHad].e() : 0.;
        eventMain[iProj].tau(dTau);

      // End of loop over interactions in a nucleus.
      }
      break; //

      event_repeat:;
    } while (unsuccessful_iterations < 100);

    if (unsuccessful_iterations >= 100) {
      CORSIKA_LOG_ERROR("Pythia event generation failed after 100 trials; returning without secondaries");
      return;
    } 


    MomentumVector Plab_final{labFrameBoost.getOriginalCS()};
    auto Elab_final = HEPEnergyType::zero();

    for (Pythia8::Particle const& p8p : eventMain) {
      // skip particles that have decayed / are initial particles in pythia's event record
      if (!p8p.isFinal()) continue;
      try {
      auto const volatile id = static_cast<PDGCode>(p8p.id());
      auto const pyId = convert_from_PDG(id);

      MomentumVector const pyPlab(rotCS,
                                  {p8p.px() * 1_GeV, p8p.py() * 1_GeV, p8p.pz() * 1_GeV});
      auto const pyP = labFrameBoost.fromCoM(FourVector{p8p.e() * 1_GeV, pyPlab});

      HEPEnergyType const mass = get_mass(pyId);
      HEPEnergyType const Ekin = sqrt(pyP.getSpaceLikeComponents().getSquaredNorm() + mass * mass) - mass;

      // add to corsika stack
      auto pnew =
          projectile.addSecondary(std::make_tuple(pyId, Ekin, pyPlab.normalized()));

      Plab_final += pnew.getMomentum();
      Elab_final += pnew.getEnergy();
      } catch (std::out_of_range const& ex) {
        CORSIKA_LOG_CRITICAL("Pythia ID {} unknown in C8", p8p.id());
        throw ex;
      }
    }
    
    eventMain.clear();

    CORSIKA_LOG_DEBUG(
        "conservation (all GeV): "
        "Elab_final= {}"
        ", Plab_final= {}",
        Elab_final / 1_GeV, (Plab_final / 1_GeV).getComponents());
  }

} // namespace corsika::pythia8
