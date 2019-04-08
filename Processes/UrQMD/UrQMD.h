#ifndef _Processes_UrQMD_UrQMD_h
#define _Processes_UrQMD_UrQMD_h

#include <array>

namespace corsika::process::UrQMD {
  class UrQMD {
  public:
    UrQMD();
  };

  namespace constants {
    // from coms.f
    int constexpr nmax = 500;
    int constexpr nspl = 500;
    int constexpr smax = 500;
    // from comres.f
    int constexpr minnuc = 1;
    int constexpr minmes = 100;
    int constexpr maxmes = 132;
    int constexpr numnuc = 16;
    int constexpr numdel = 10;
    int constexpr maxnuc = minnuc + numnuc - 1;
    int constexpr mindel = minnuc + maxnuc;
    int constexpr maxdel = mindel + numdel - 1;
    int constexpr minres = minnuc + 1;
    int constexpr maxres = maxdel;
    int constexpr numlam = 13;
    int constexpr numsig = 9;
    int constexpr numcas = 6;
    int constexpr numome = 1;
    int constexpr minlam = mindel + numdel;
    int constexpr maxlam = minlam + numlam - 1;
    int constexpr minsig = minlam + numlam;
    int constexpr maxsig = minsig + numsig - 1;
    int constexpr mincas = minsig + numsig;
    int constexpr maxcas = mincas + numcas - 1;
    int constexpr minome = mincas + numcas;
    int constexpr maxome = minome + numome - 1;
    int constexpr minbar = minnuc;
    int constexpr maxbar = maxome;
    int constexpr offmeson = minmes;
    int constexpr maxmeson = maxmes;
    int constexpr maxbra = 11;
    int constexpr maxbrm = 25;
    int constexpr maxbrs1 = 10;
    int constexpr maxbrs2 = 3;
    int constexpr nsigs = 10;
    int constexpr itblsz = 100;
    int constexpr maxreac = 13;
    int constexpr maxpsig = 12;

    // from comwid.f
    int constexpr widnsp = 120;
    double constexpr mintab = 0.10;
    double constexpr maxtab1 = 5.0;
    double constexpr maxtab2 = 50.0;
    int constexpr tabver = 9;

    // from options.f
    int constexpr numcto = 400;
    int constexpr numctp = 400;
    int constexpr maxstables = 20;

    // from colltab.f
    int constexpr ncollmax = 100;

    // from inputs.f
    int constexpr aamax = 300;

    // from newpart.f
    int constexpr mprt = 200;
    int constexpr oprt = 2;

    // from boxinc.f
    int constexpr bptmax = 20;

    // from comnorm.f
    int constexpr n = 400;

    // from comstr.f
    int constexpr njspin = 8;

    // iso
    int constexpr jmax = 7;
  } // namespace constants

  using nmaxIntArray = std::array<int, constants::nmax>;
} // namespace corsika::process::UrQMD

extern "C" {
void iniurqmd_();
double ranf_(int*);

struct {
  int npart, nbar, nmes, ctag, nsteps, uid_cnt, ranseed, event, Ap, At, Zp, Zt, eos,
      dectag, NHardRes, NSoftRes, NDecRes, NElColl, NBlColl;
} sys_;

struct {
  double time, acttime, bdist, bimp, bmin, ebeam, ecm;
} rsys_;

struct {
  int firstseed;
} comseed_;

struct {
  corsika::process::UrQMD::nmaxIntArray lsct;
  int logSky, logYuk, logCb, logPau;
} logic_;

struct {
  corsika::process::UrQMD::nmaxIntArray spin, ncoll, charge, ityp, lstcoll, iso3, origin, strid, uid;
} isys_;
}

#endif
