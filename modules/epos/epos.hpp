/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <array>
#include <string>

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
  extern float rndm_interface();

  /**
   * \function epos::double_rndm_interface
   *
   * this is the random number hook to external packages.
   *
   * CORSIKA8, for example, has to provide an implementation of this.
   **/

  extern double double_rndm_interface();

  extern "C" {

  void aaset_(int&);
  void atitle_();
  void ainit_();
  void aepos_(int&);
  void afinal_();
  void alistf_(char[7]);
  double lhcparameters_();
  void hdecin_(bool&);
  void hnbspd_(int&);
  void hnbpajini_();
  void conini_();
  void psaini_();

  void emsini_(double&, int&, int&);
  void paramini_(int&);
  void xsigma_();

  //
  //  cross section from tables
  //
  // c------------------------------------------------------------------------------
  //   function eposcrse(ek,mapro,matar,id)
  // c------------------------------------------------------------------------------
  // c inelastic cross section of epos
  // c (id=0 corresponds to air)
  // c ek     - kinetic energy for the interaction in the lab
  // c maproj - projec mass number     (1<maproj<64)
  // c matarg - target mass number     (1<matarg<64)
  // c------------------------------------------------------------------------------
  float eposcrse_(float&, int&, int&, int&);
  float eposelacrse_(float&, int&, int&, int&);

  // calculate cross section
  // c------------------------------------------------------------------------------
  //       subroutine crseaaEpos(sigt,sigi,sigc,sige)
  // c------------------------------------------------------------------------------
  // c nucleus-nucleus (hadron) cross section of epos from simplified (realistic)
  // c simulations
  // c (id=0 corresponds to air)
  // c  sigt = sig tot
  // c  sigi = sig inelastic (cut + projectile diffraction)
  // c  sigc = sig cut
  // c  sige = sig elastic (includes target diffraction)
  // c------------------------------------------------------------------------------
  void crseaaepos_(float&, float&, float&, float&);

  void emsaaa_(int&);
  void gakfra_(int&, int&);
  void utghost_(int&);
  void bjinta_(int&);
  void utresc_(int&);

  void emsfrag_(int&);

  // get particles hadron class: meson, baryon etc..???
  void iclass_(int&, int&);
  // get charge for id
  void idchrg_(int&, int&);
  // get isospin, spin, strangeness for id
  void idspin_(int&, int&, int&, int&);
  // get mass for id
  void idmass_(int&, float&);
  // convert id from one format to another
  int idtrafo_(char[3], char[3], int&);

  // additional random number functions
  void ranfini_(double&, int&, int&);
  void ranfcv_(double&);

  float rangen_();
  double drangen_();
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
    int iapplxs;
    int modelxs;
  } xsappli_;

  extern struct {
    int nevent;
    int nfull;
    int nfreeze;
    int ninicon;
  } events_;

  extern struct {
    int neventxs;
    int iframexs;
  } xsevent_;

  //   common/metr1/iospec,iocova,iopair,iozero,ioflac,iomom
  extern struct {
    int iospec;
    int iocova;
    int iopair;
    int iozero;
    int ioflac;
    int iomom;
  } metr1_;

  extern struct {
    int ifrade;
    int iframe;
    int idecay;
    int jdecay;
    int iremn;
  } othe2_;

  extern struct { int ktnbod; } metr7_;

  extern struct {
    float egylow;
    float egyfac;
  } had12_;

  extern struct {
    int laproj;
    int maproj;
    int latarg;
    int matarg;
    float core;
    float fctrmx;
  } nucl1_;

  extern struct {
    float amproj;
    float amtarg;
    float ypjtl;
    float yhaha;
    float pnullx;
  } chadron_;

  extern struct {
    int iomodl;
    int idproj;
    int idtarg;
    float wexcit;
  } hadr2_;

  extern struct {
    int idprojin;
    int idtargin;
    float rexdifi[4];
    float rexndii[4];
    int irdmpr;
    int isoproj;
    int isotarg;
  } hadr25_;

  extern struct {
    float engy;
    float elepti;
    float elepto;
    float angmue;
    int icinpu;
  } lept1_;

  extern struct {
    float egymin;
    float egymax;
    float elab;
    float ecms;
    float ekin;
  } enrgy_;

  extern struct {
    float pnll;
    float ptq;
    float exmass;
    float cutmss;
    float wproj;
    float wtarg;
  } hadr1_;

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

  extern struct {
    double seedi;
    double seedj;
    double seedj2;
    double seedc;
    int iseqini;
    int iseqsim;
  } cseed_;

  extern struct {
    int istore;
    int istmax;
    int gaumx;
    int irescl;
    int ntrymx;
    int nclean;
    int iopdg;
    int ioidch;
  } othe1_;

  extern struct {
    int ifop;
    int ifmt;
    int ifch;
    int ifcx;
    int ifhi;
    int ifdt;
    int ifcp;
    int ifdr;
  } files_;

  extern struct {
    char fnch[500];
    char fnhi[500];
    char fndt[500];
    char fnii[500];
    char fnid[500];
    char fnie[500];
    char fnrj[500];
    char fnmt[500];
    char fngrv[500];
    char fncp[500];
    char fnnx[500];
    char fncs[500];
    char fndr[500];
    char fnhpf[500];

  } fname_;

  extern struct {
    int nfnch;
    int nfnhi;
    int nfndt;
    int nfnii;
    int nfnid;
    int nfnie;
    int nfnrj;
    int nfnmt;
    int nfngrv;
    int nfncp;
    int nfnnx;
    int nfncs;
    int nfndr;
    int nfnhpf;
  } nfname_;

  extern struct {
    int iprmpt;
    int ish;
    int ishsub;
    int irandm;
    int irewch;
    int iecho;
    int modsho;
    int idensi;
  } prnt1_;
  unsigned int constexpr mmry = 1;
  unsigned int constexpr mxptl = 200000 / mmry;
  extern struct {
    int nptl;
    float pptl[mxptl][5];
    int iorptl[mxptl];
    int idptl[mxptl];
    int istptl[mxptl];
    float tivptl[mxptl][2];
    int ifrptl[mxptl][2];
    int jorptl[mxptl];
    float xorptl[mxptl][4];
    int ibptl[mxptl][4];
    int ityptl[mxptl];
  } cptl_;

  extern struct {
    float sigtot;
    float sigcut;
    float sigela;
    float sloela;
    float sigsd;
    float sigine;
    float sigdif;
    float sigineaa;
    float sigtotaa;
    float sigelaaa;
    float sigcutaa;
    float sigdd;
  } hadr5_;

  unsigned int constexpr mxnody = 200;
  extern struct {
    int nrnody;
    int nody[mxnody];
  } nodcy_;

  extern struct {
    int iclpro;
    int icltar;
    int iclegy;
  } had10_;

  /**
   Small helper class to provide a data-directory name in the format eposlhc expects
  */
  class datadir {
  private:
    datadir operator=(const std::string& dir);
    datadir operator=(const datadir&);

  public:
    datadir(const std::string& dir);
    char data[500];
    int length;
  };
  }
} // namespace epos
