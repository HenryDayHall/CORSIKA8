/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_particle_cut_CONEXSourceCut_h_
#define _corsika_process_particle_cut_CONEXSourceCut_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace conex {
  extern "C" {
  extern struct cxcut_ {
    double eecut;
    double epcut;
    double ehcut;
    double emcut;
  };

  extern struct cxsubcut_ {
    double feecut_;
    double fehcut_;
    double femcut_;
  };

  extern struct cxbas4_ {
    double eprima_;
    double thetas_;
    double costhet_;
    double phisho_;
    int muse;
    int musz;
    double c2bas;
    double sinthet;
    double sinphi;
    double XminSlant;
    double HGrd;
    double distMaxi;
  };

  extern struct cxoptl_ {
    struct dptl_ {
      double px, py, pz, E, m; // 5-momentum
      double x, y;
      double h;
      double t;
      double id;
      double weight;
      double generation;
      double Xslant;
      double lateralX, lateralY;
      double slantDistance;
    };
  };

  extern struct cxbas6_ {
    double altitude;
    double RadGrd;
    double DistALt;
    double dphmaxi0;
    double dphlim0;
    double dphmin0;
    bool goOutGrd;
  };

  extern struct cxetc_ {
    int mode;
    int iwrt;
    int i1DMC;
    int iphonu;
  };

  extern struct cxthin_ {
    double thin;
    double ethin;
    double wtmax;
    double rthmax;
    int iothin;
  };

  extern struct cxoutput3_ {
    std::array<std::array<double, 4>, mxPxpro + 2> XmaxShow;
    std::array<std::array<double, 5>, mxPxpro + 2> XmaxMean;
    std::array<std::array<double, maximz>, mxPxpro> XmaxProf;
    int iXmax;
  };

  int constexpr mxExpro = 1;
  int constexpr mxPxpro = 10,

                int constexpr maximE = 281;

// CONEX ifdef
#ifdef __SAVEMEMO__
  int constexpr maximZ = 201;
#else
  int constexpr maximZ = 3701;
#endif

  extern struct cxoutput1_ {
    std::array<std::array<std::array<double, maximZ>, mxExpro>, mxPxpro + 2> XProf,
        XmeanP, XmeanP2;
    std::array<double, mxExpro> EMCutP, HaCutP;
    double XminP;
    double XmaxP;
    int nminX;
    int nmaxX;
    int mZEMHa;
    int ifout;
    int ivers;
    bool lheader;
  };

  /*
          common/cxoutput3/XmaxShow(4,-1:mxPxpro),XmaxMean(5,-1:mxPxpro)
       &,XmaxProf(maximz,-1:mxPxpro),iXmax
        common/cxoutput1/XProf(maximz,mxExpro,-1:mxPxpro)
       &,XmeanP(maximz,mxExpro,-1:mxPxpro)
       &,XmeanP2(maximz,mxExpro,-1:mxPxpro)
       &,EMCutP(mxExpro),HaCutP(mxExpro),XminP,XmaxP
       &,nminX,nmaxX,mZEMHa,ifout,ivers,lheader
  */

  int InitialParticle_(int&);
  void HadronCascade_(int&, int&, int&, int&);
  void SolveMomentEquations_(int&);
  void show_(int& iqi, double& ei, double& xmi, double& ymi, double& zmi, double& dmi,
             double& xi, double& yi, double& zi, double& tmi, double& ui, double& vi,
             double& wi, int& iri, double& wti, int& latchi);
  }

  corsika::units::si::LengthType constexpr earthRadius{6371315 *
                                                       corsika::units::si::meter};
} // namespace conex

namespace corsika::process {
  namespace conex_source_cut {
    class CONEXSourceCut : public process::SecondariesProcess<CONEXSourceCut> {

    public:
      CONEXSourceCut(geometry::Point const& center, environment::ShowerAxis const&,
                     units::si::LengthType, units::si::GrammageType);
      corsika::process::EProcessReturn DoSecondaries(corsika::setup::StackView&);

      void Init();

      void SolveCE();

    private:
      //! CONEX e.m. particle codes
      static std::array<std::pair<particles::Code, int>, 3> constexpr egs_em_codes_{
          {particles::Code::Gamma, 0},
          {particles::Code::Electron, -1},
          {particles::Code::Positron, -1}};

      int egs_calls_{1};
      int nshtot_{1};

      units::si::LengthType groundDist_; //!< length from injection point to shower core
      geometry::Point const center_;     //!< center of CONEX Earth
      environment::ShowerAxis const& showerAxis_;
      geometry::CoordinateSystem const conexObservationCS_; //!< CONEX observation frame
      Vector<units::si::dimensionless_d> const x_sf_,
          y_sf_; //!< unit vectors of CONEX shower frame, z_sf is shower axis direction
    };
  } // namespace conex_source_cut
} // namespace corsika::process

#endif
