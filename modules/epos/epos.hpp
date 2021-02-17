/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <array>

/**
 * \file epos.hpp
 *
 * Interface file for the EPOS library.
 */

namespace epos {
  /**
   * \function epos::rndm_interface
   *
   * this is the random number hook to external packages.
   *
   * CORSIKA8, for example, has to provide an implementation of this.
   **/
  extern double rndm_interface();

  extern "C" {

  void aaset_(int&);
  void atitle_();
  double LHCparameters_();
  void hdecin_(bool&);
  void hnbspd_(int&);
  void hnbpajini_();
  void conini_();
  void psaini_();

  void idspin_(int&, int&, int&, int&);
  void iclass_(int&, int&);
  void emsini_(double&, int&, int&);
  void paramini_(int&);
  void xsigma_();

  double cxepocrse_(double&, int&, int&, int&);

  void emsaaa_(int&);
  void gakfra_(int&, int&);
  void utghost_(int&);
  void bjinta_(int&);
  void utresc_(int&);

  void emsfrag_(int&);

  void cxidmass_(int&, int&);

  // additional random number functions
  void ranfini_(double&, int&, int&);
  void ranfcv_(double&);
  // void ranfgt(int&);
  double rangen_();

  // common blocks as
  // defined in epos.inc

  extern struct { int inicnt; } cicnt_;

  extern struct {
    int intpol;
    int isigma;
    int iomega;
    int isetcs;
  } hadr6_;

  extern struct { int infragm; } nucl6_;

  extern struct {
    int iorsce;
    int iorsdf;
    int iorshh;
    int ionudi;
  } cjinti_;

  extern struct {
    float airznxs[3];
    float airanxs[3];
    float airwnxs[3];
    float airavznxs;
    float airavanxs;
  } nxsair_;

  extern struct {
    int iappl;
    int model;
  } appli_;

  extern struct {
    int nevent;
    int nfull;
    int nfreeze;
    int ninicon;
  } events_;

  extern struct {
    int ifrade;
    int iframe;
    int idecay;
    int jdecay;
    int iremn;
  } othe2_;
  // integer      ifrade,iframe,idecay,jdecay,iremn
  // common/othe2/ifrade,iframe,idecay,jdecay,iremn

  extern struct { int ktnbod; } metr7_;
  // integer      ktnbod
  // common/metr7/ktnbod

  extern struct {
    float egylow;
    float egyfac;
  } had12_;
  //      real         egylow,egyfac
  // common/had12/egylow,egyfac

  extern struct {
    int laproj;
    int maproj;
    int latarg;
    int matarg;
    float core;
    float fctrmx;
  } nucl1_;
  // real         core,fctrmx
  // integer       laproj,maproj,latarg,matarg
  // common/nucl1/laproj,maproj,latarg,matarg,core,fctrmx

  extern struct {
    float amproj;
    float amtarg;
    float ypjtl;
    float yhaha;
    float pnullx;
  } chadron_;
  // real           amproj,amtarg,ypjtl,yhaha,pnullx
  // common/chadron/amproj,amtarg,ypjtl,yhaha,pnullx

  extern struct {
    int iomodl;
    int idproj;
    int idtarg;
    float wexcit;
  } hadr2_;
  // integer      iomodl,idproj,idtarg
  // real         wexcit
  // common/hadr2/iomodl,idproj,idtarg,wexcit

  extern struct {
    int idprojin;
    int idtargin;
    float rexdifi[4];
    float rexndii[4];
    int irdmpr;
    int isoproj;
    int isotarg;
  } hadr25_;

  //       real          rexdifi,rexndii
  // integer       idprojin,idtargin,irdmpr,isoproj,isotarg
  // common/hadr25/idprojin,idtargin,rexdifi(4),rexndii(4),irdmpr,
  // *              isoproj,isotarg

  extern struct {
    float engy;
    float elepti;
    float elepto;
    float angmue;
    int icinpu;
  } lept1_;
  //  real engy, elepti, elepto, angmue integer icinpu
  // common / lept1 / engy, elepti, elepto,
  // angmue,
  // icinpu

  extern struct {
    float egymin;
    float egymax;
    float elab;
    float ecms;
    float ekin;
  } enrgy_;
  // real egymin, egymax, elab, ecms, ekin
  // common / enrgy / egymin, egymax, elab, ecms,
  // ekin

  extern struct {
    float pnll;
    float ptq;
    float exmass;
    float cutmss;
    float wproj;
    float wtarg;
  } hadr1_;
  // real pnll, ptq, exmass, cutmss, wproj, wtarg
  // common / hadr1 / pnll, ptq, exmass, cutmss,
  // wproj,
  // wtarg

  unsigned int constexpr idxD0 = 0;
  unsigned int constexpr idxD1 = 2;
  unsigned int constexpr idxD = 1;
  unsigned int constexpr nclha = 4;
  unsigned int constexpr nclegy = 100;

  extern struct {
    float alpD[nclha][nclha][idxD1 - idxD0 + 1];
    float alpdp[nclha][nclha][idxD1 - idxD0 + 1];
    float alpDpp[nclha][nclha][idxD1 - idxD0 + 1];
    float betD[nclha][nclha][idxD1 - idxD0 + 1];
    float betDp[nclha][nclha][idxD1 - idxD0 + 1];
    float betDpp[nclha][nclha][idxD1 - idxD0 + 1];
    float gamD[nclha][nclha][idxD1 - idxD0 + 1];
    float delD[nclha][nclha][idxD1 - idxD0 + 1];
    int idxDmin;
    float bmxdif[nclha][nclha];
    float bkmxndif;
  } Dparam_;
  //  real bmxdif,bkmxndif
  // integer idxDmin
  // common / Dparam / alpD(idxD0: idxD1, nclha, nclha),
  //* alpDp(idxD0 : idxD1, nclha, nclha),
  //*alpDpp(idxD0 : idxD1, nclha, nclha),
  //*  betD(idxD0 : idxD1, nclha, nclha),
  //* betDp(idxD0 : idxD1, nclha, nclha),
  //*betDpp(idxD0 : idxD1, nclha, nclha),
  //*  gamD(idxD0 : idxD1, nclha, nclha),
  //*  delD(idxD0 : idxD1, nclha, nclha),
  //*idxDmin, bmxdif(nclha, nclha),  bkmxndif

  extern struct {
    float phievt;
    int nevt;
    float bimevt;
    int kolevt;
    int koievt;
    float pmxevt;
    float egyevt;
    int npjevt;
    int ntgevt;
    int npnevt;
    int nppevt;
    int ntnevt;
    int ntpevt;
    int jpnevt;
    int jppevt;
    int jtnevt;
    int jtpevt;
    float xbjevt;
    float qsqevt;
    int nglevt;
    float zppevt;
    float zptevt;
    int minfra;
    int maxfra;
    int kohevt;
  } cevt_;
  //  real phievt, bimevt, pmxevt, egyevt , xbjevt, qsqevt, zppevt, zptevt
  // integer nevt,
  // kolevt, koievt, kohevt, npjevt , ntgevt, npnevt, nppevt, ntnevt, ntpevt, jpnevt,
  // jppevt, jtnevt, jtpevt , nglevt, minfra, maxfra
  // common / cevt / phievt, nevt,
  //    bimevt, kolevt, koievt, pmxevt, egyevt, npjevt , ntgevt, npnevt, nppevt, ntnevt,
  //    ntpevt, jpnevt, jppevt, jtnevt, jtpevt , xbjevt, qsqevt, nglevt, zppevt, zptevt,
  //    minfra, maxfra, kohevt
  }
} // namespace epos
