#pragma once

//----------------------------------------------
//  C++ interface for the CONEX fortran code
//----------------------------------------------
// wrapper

extern "C" {
  struct cxcut_ {
    double eecut;
    double epcut;
    double ehcut;
    double emcut;
  };

  struct cxsubcut_ {
    double feecut_;
    double fehcut_;
    double femcut_;
  };

  struct cxbas4_ {
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

  struct cxoptl_ {
  //struct cxoptl_ {
  //  std::array<double, 16> dptl;
  //};

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

  struct cxbas6_ {
    double altitude;
    double RadGrd;
    double DistALt;
    double dphmaxi0;
    double dphlim0;
    double dphmin0;
    bool goOutGrd;
  };

  struct cxetc_ {
    int mode;
    int iwrt;
    int i1DMC;
    int iphonu;
  };

  struct cxthin_ {
    double thin;
    double ethin;
    double wtmax;
    double rthmax;
    int iothin;
  };

  int constexpr mxExpro = 1;
  int constexpr mxPxpro = 10;
  int constexpr maximE = 281;
// CONEX ifdef
#ifdef __SAVEMEMO__
  int constexpr maximZ = 201;
#else
  int constexpr maximZ = 3701;
#endif

  struct cxoutput3_ {
    std::array<std::array<double, 4>, mxPxpro + 2> XmaxShow;
    std::array<std::array<double, 5>, mxPxpro + 2> XmaxMean;
    std::array<std::array<double, maximZ>, mxPxpro> XmaxProf;
    int iXmax;
  };

  struct cxoutput1_ {
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

  int get_number_of_depth_bins_(); 

    void get_shower_data_(const int&, const int&, const int&, float&, float&,
			float&, float&, float&);
    void get_shower_edep_(const int&, const int&, float&, float&);
    void get_shower_muon_(const int&, const int&, float&, float&);
    void get_shower_gamma_(const int&, const int&, float&);
    void get_shower_electron_(const int&, const int&, float&);
    void get_shower_hadron_(const int&, const int&, float&);

  }

