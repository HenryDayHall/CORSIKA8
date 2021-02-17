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
//#include <corsika/modules/sibyll/ParticleConversion.hpp>
//#include <corsika/modules/sibyll/SibStack.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <epos.hpp>

#include <tuple>

using namespace corsika;
using SetupParticle = setup::Stack::stack_iterator_type;

namespace corsika::epos {

  inline Interaction::Interaction() {

    //       if(ilowegy.ne.1.or.MCleModel.eq.4)xsegymin=dble(0.5*egymin**2)
    //       if(MCModel.eq.4)xsegymax=min(xsegymax,dble(0.5*egymax**2))
    //       nrnody=nrnodyxs
    //       do i=1,nrnody
    //         nody(i)= nodyxs(i)
    //       enddo
    // #if !__CXCORSIKA__ && !__CORSIKA8__

    //       inicnt=inicnt+1
    // c      isetcs=2  ! epos cross-section from tabulated calculation (h-A and AA)
    ::epos::hadr6_.isetcs=3; //  epos cross-section from tabulated simulations
                             //       (h-A and A-A)
    ::epos::nucl6_.infragm=2; // --> model how projectiles fragment!
    ::epos::hadr6_.isigma=0; //                  !do not print out the
    //       cross section on screen ionudi=1

    //       nfnii=nxsfnii             ! epos file name
    //       fnii=xsfnii
    //       nfnid=nxsfnid
    //       fnid=xsfnid
    //       nfnie=nxsfnie
    //       fnie=xsfnie
    //       nfnrj=nxsfnrj
    //       fnrj=xsfnrj
    //       nfncs=nxsfncs
    //       fncs=xsfncs
    //       nfnch=nxsfnch
    //       fnch=xsfnch

    // c air
    /*    for (int i=1; i<=3; ++i) {
      ::epos::nxsair_.airanxs[i]=aira[i];
      ::epos::nxsair_.airznxs[i]=airz[i];
      ::epos::nxsair_.airwnxs[i]=airw[i];
	}
    ::epos::nxsair_.airavanxs=airava;
    ::epos::nxsair_.airavznxs=airavz;
    */

    ::epos::appli_.iappl = iapplxs;
    ::epos::events_.nevent = neventxs;
    ::epos::othe2_.iframe = iframexs;

    //       if(fnch(1:nfnch).ne.'none')
    //      &  open(ifcx,file=fnch(1:nfnch),status='unknown')

    //       call iclass(idproj,iclpro)
    //       call iclass(idtarg,icltar)
    //       if(inicnt.eq.1)then
    //         call ranfgt(seedp)      !not to change the seed ...
    ::epos::hdecin_(false);
    ::epos::hnbspd_(iospec);
    //         ktnbod=0
    ::epos::hnbpajini_();
    //         if(iclegy2.gt.1)then
    //           egyfac=(egymax*1.0001/egylow)**(1./float(iclegy2-1))
    //         else
    //           egyfac=1.
    //         endif
    //       endif
    //       maproj=mamx               !to set difnuc up to the maximum mass
    //       call conini
    //       call psaini
    //       call ranfst(seedp)        ! ... after this initialization
  }

  inline Interaction::~Interaction() {
    CORSIKA_LOG_DEBUG("Epos::Interaction n={} ", count_);
  }

  inline corsika::CrossSectionType Interaction::getCrossSection(
      const corsika::Code BeamId, const corsika::Code TargetId,
      const corsika::HEPEnergyType CoMenergy) const {}

  template <>
  inline corsika::GrammageType Interaction::getInteractionLength(
      SetupParticle const& projectile) const {

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function SIBYLL is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <typename TSecondaryView>
  inline void Interaction::doInteraction(TSecondaryView& view) {

    auto const projectile = view.getProjectile();
    const auto corsikaBeamId = projectile.getPID();

    if (corsika::is_nucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by SIBYLL!");
    }

    // position and time of interaction, not used in Sibyll
    Point const pOrig = projectile.getPosition();
    TimeType const tOrig = projectile.getTime();

    // define projectile
    HEPEnergyType const eProjectileLab = projectile.getEnergy();
    auto const pProjectileLab = projectile.getMomentum();
    CoordinateSystemPtr const& originalCS = pProjectileLab.getCoordinateSystem();

    CORSIKA_LOG_DEBUG(
        "ProcessEPOS: "
        "DoInteraction: pid {} interaction ",
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
      auto const sigProd = getCrossSection(corsikaBeamId, targetId, Ecm);
      cross_section_of_components[i] = sigProd;
    }

    const auto targetCode =
        mediumComposition.sampleTarget(cross_section_of_components, RNG_);
    CORSIKA_LOG_DEBUG("Interaction: target selected: {} ", targetCode);

    //

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
