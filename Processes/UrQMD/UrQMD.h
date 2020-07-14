/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/InteractionProcess.h>
#include <corsika/random/RNGManager.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/CorsikaData.h>

#include <boost/multi_array.hpp>

#include <array>
#include <random>
#include <string>
#include <utility>

namespace corsika::process::UrQMD {
  class UrQMD : public corsika::process::InteractionProcess<UrQMD> {
  public:
    UrQMD(std::string const& path = utl::CorsikaData("UrQMD/UrQMD-1.3.1-xs.dat"));
    corsika::units::si::GrammageType GetInteractionLength(
        corsika::setup::Stack::StackIterator const&) const;

    template <typename TParticle>
    corsika::units::si::CrossSectionType GetCrossSection(TParticle const&,
                                                         corsika::particles::Code) const;

    corsika::units::si::CrossSectionType GetCrossSection(
        particles::Code, particles::Code, corsika::units::si::HEPEnergyType,
        int Ap = 1) const;

    corsika::units::si::CrossSectionType GetTabulatedCrossSection(
        particles::Code, particles::Code, corsika::units::si::HEPEnergyType) const;

    corsika::process::EProcessReturn DoInteraction(
        corsika::setup::StackView::StackIterator&);

    bool CanInteract(particles::Code) const;

  private:

    void readXSFile(std::string const&);

    corsika::random::RNG& rng_ =
        corsika::random::RNGManager::GetInstance().GetRandomStream("urqmd");

    std::uniform_int_distribution<int> booleanDist_{0, 1};
    boost::multi_array<corsika::units::si::CrossSectionType, 3> xs_interp_support_table_;
  };

  namespace details::constants {
    // from coms.f
    int constexpr nmax = 500;

    // from options.f
    int constexpr numcto = 400;
    int constexpr numctp = 400;

    // from inputs.f
    int constexpr aamax = 300;

  } // namespace details::constants

  template <typename T>
  using nmaxArray = std::array<T, details::constants::nmax>;
  using nmaxIntArray = nmaxArray<int>;
  using nmaxDoubleArray = nmaxArray<double>;

  extern "C" {
  // FORTRAN functions defined in UrQMD
  void iniurqmdc8_();
  double ranf_(int&);
  void cascinit_(int const&, int const&, int const&);
  double nucrad_(int const&);
  void urqmd_(int&);
  int pdgid_(int const&, int const&);
  double sigtot_(int&, int&, double&);
  int collclass_(int&, int&, int&, int&);
  double crossx_(int const&, double const&, int const&, int const&, double const&,
                 int const&, int const&, double const&, double&);
  int readsigmaln_(int const&, int const&, int const&);

  // defined in coms.f
  extern struct {
    int npart, nbar, nmes, ctag, nsteps, uid_cnt, ranseed, event;
    int Ap; // projectile mass number (in case of nucleus)
    int At; // target mass number (in case of nucleus)
    int Zp; // projectile charge number (in case of nucleus)
    int Zt; // target charge number (in case of nucleus)
    int eos, dectag, NHardRes, NSoftRes, NDecRes, NElColl, NBlColl;
  } sys_;

  extern struct {
    double time, acttime, bdist, bimp, bmin;
    double ebeam; // lab-frame energy of projectile
    double ecm;
  } rsys_;

  // defined in coms.f
  extern struct {
    nmaxIntArray spin, ncoll, charge, ityp, lstcoll, iso3, origin, strid, uid;
  } isys_;

  // defined in coor.f
  extern struct {
    nmaxDoubleArray r0, rx, ry, rz, p0, px, py, pz, fmass, rww, dectime;
  } coor_;

  // defined in inputs.f
  extern struct {
    int nevents;
    std::array<int, 2> spityp; // particle codes of: [0]: projectile, [1]: target
    int prspflg;               // projectile special flag
    int trspflg; // target special flag, set to 1 unless target is nucleus > H
    std::array<int, 2> spiso3; // particle codes of: [0]: projectile, [1]: target
    int outsteps, bflag, srtflag, efuncflag, nsrt, npb, firstev;
  } inputs_;

  // defined in inputs.f
  extern struct {
    double srtmin, srtmax, pbeam, betann, betatar, betapro, pbmin, pbmax;
  } input2_;

  // defined in options.f
  extern struct {
    std::array<double, details::constants::numcto> CTOption;
    std::array<double, details::constants::numctp> CTParam;
  } options_;

  extern struct {
    int fixedseed, bf13, bf14, bf15, bf16, bf17, bf18, bf19, bf20;
  } loptions_;

  // defined in urqmdInterface.F
  extern struct { std::array<double, 3> xs, bim; } cxs_u2_;
  }

  /**
   * convert CORSIKA code to UrQMD code tuple
   *
   * In the current implementation a detour via the PDG code is made.
   */
  std::pair<int, int> ConvertToUrQMD(particles::Code);
  particles::Code ConvertFromUrQMD(int vItyp, int vIso3);

} // namespace corsika::process::UrQMD
