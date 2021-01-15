/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/pythia8/Pythia8.hpp>
#include <corsika/modules/pythia8/Decay.hpp>
#include <corsika/modules/pythia8/Random.hpp>

#include <corsika/framework/utility/COMBoost.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika::pythia8 {

  Decay::Decay(const bool print_listing)
      : print_listing_(print_listing) {
    init();
  }

  Decay::Decay(std::set<Code> const& those)
      : handleAllDecays_(false)
      , handledDecays_(those) {
    init();
  }

  Decay::~Decay() { CORSIKA_LOG_INFO("Pythia::Decay n={}", count_); }

  void Decay::init() {

    // run this only once during construction

    // set random number generator in pythia
    Pythia8::RndmEngine* rndm = new corsika::pythia8::Random();
    pythia_.setRndmEnginePtr(rndm);

    /*
       issue xyz: definition of particles and decay channels use the same mechanism in
       corsika and pythia we should force pythia to use the file in corsika.
     */
    // bool ParticleData::reInit(string startFile, bool xmlFormat = true)
    // read in particle data from Corsika 8
    // pythia_.particleData.reInit("/home/felix/ngcorsika/corsika-build/include/corsika/particles/ParticleData.xml");
    // pythia_.particleData.checkTable();

    pythia_.readString("Next:numberShowInfo = 0");
    pythia_.readString("Next:numberShowProcess = 0");
    pythia_.readString("Next:numberShowEvent = 0");

    pythia_.readString("Print:quiet = on");
    pythia_.readString("Check:particleData = 0");

    /*
       switching off event check in pythia is needed to allow decays that are off-shell
       according to the mass definition in pythia.
       the consistency of particle masses between event generators is an unsolved issues
    */
    CORSIKA_LOG_INFO("Pythia::Init: switching off event checking in pythia..");
    pythia_.readString("Check:event = 1");

    pythia_.readString("ProcessLevel:all = off");
    pythia_.readString("ProcessLevel:resonanceDecays = off");

    // making sure
    setStable(Code::Pi0);

    //    pythia_.particleData.readString("59:m0 = 101.00");

    if (!pythia_.init())
      throw std::runtime_error("Pythia::Decay: Initialization failed!");
  }

  bool Decay::canHandleDecay(Code const vParticleCode) {
    // if known to pythia and not proton, electron or neutrino it can decay
    if (vParticleCode == Code::Proton || vParticleCode == Code::AntiProton ||
        vParticleCode == Code::NuE || vParticleCode == Code::NuMu ||
        vParticleCode == Code::NuTau || vParticleCode == Code::NuEBar ||
        vParticleCode == Code::NuMuBar || vParticleCode == Code::NuTauBar ||
        vParticleCode == Code::Electron || vParticleCode == Code::Positron)
      return false;
    else if (canDecay(vParticleCode)) // non-zero for particles known to sibyll
      return true;
    else
      return false;
  }

  void Decay::setHandleDecay(Code const vParticleCode) {
    handleAllDecays_ = false;
    CORSIKA_LOG_INFO("Pythia::Decay: set to handle decay of {} ", vParticleCode);
    if (Decay::canHandleDecay(vParticleCode))
      handledDecays_.insert(vParticleCode);
    else
      throw std::runtime_error("this decay can not be handled by pythia!");
  }

  void Decay::setHandleDecay(std::vector<Code> const& vParticleList) {
    handleAllDecays_ = false;
    for (auto p : vParticleList) setHandleDecay(p);
  }

  bool Decay::isDecayHandled(Code const vParticleCode) {
    if (handleAllDecays_ && canHandleDecay(vParticleCode))
      return true;
    else
      return handledDecays_.find(vParticleCode) != Decay::handledDecays_.end();
  }

  void Decay::setStable(std::vector<Code> const& particleList) {
    for (auto p : particleList) Decay::setStable(p);
  }

  void Decay::setUnstable(Code const pCode) {
    CORSIKA_LOG_INFO("Pythia::Decay: setting {} unstable..", pCode);
    pythia_.particleData.mayDecay(static_cast<int>(get_PDG(pCode)), true);
  }

  void Decay::setStable(Code const pCode) {
    CORSIKA_LOG_INFO("Pythia::Decay: setting {} stable..", pCode);
    pythia_.particleData.mayDecay(static_cast<int>(get_PDG(pCode)), false);
  }

  bool Decay::isStable(Code const vCode) {
    return pythia_.particleData.canDecay(static_cast<int>(get_PDG(vCode)));
  }

  bool Decay::canDecay(Code const pCode) {
    const bool ans = pythia_.particleData.canDecay(static_cast<int>(get_PDG(pCode)));
    CORSIKA_LOG_INFO("Pythia::Decay: checking if particle: {} can decay in PYTHIA? {} ",
                     pCode, ans);
    return ans;
  }

  void Decay::printDecayConfig(const Code vCode) {
    CORSIKA_LOG_INFO("Decay: Pythia decay configuration:");
    CORSIKA_LOG_INFO(" {} is {} ", vCode, (isStable(vCode) ? "stable" : "unstable"));
  }

  void Decay::printDecayConfig() {
    CORSIKA_LOG_INFO("Pythia::Decay: decay configuration:");
    if (handleAllDecays_)
      CORSIKA_LOG_INFO(" all particles known to Pythia are handled by Pythia::Decay!");
    else
      for (auto& pCode : handledDecays_)
        CORSIKA_LOG_INFO("Decay of {} is handled by Pythia!", pCode);
  }

  template <typename TParticle>
  TimeType Decay::getLifetime(TParticle const& particle) {

    const auto pid = particle.getPID();
    if (canDecay(pid)) {
      HEPEnergyType E = particle.getEnergy();
      HEPMassType m = particle.getMass();

      const double gamma = E / m;

      const TimeType t0 = get_lifetime(pid);
      auto const lifetime = gamma * t0;
      CORSIKA_LOG_INFO("Pythia::Decay: code: {}", particle.getPID());
      CORSIKA_LOG_INFO("Pythia::Decay: MinStep: t0: {}", t0);
      CORSIKA_LOG_INFO("Pythia::Decay: MinStep: energy: {} GeV", E / 1_GeV);
      CORSIKA_LOG_INFO("Pythia::Decay: momentum: {} GeV",
                       particle.getMomentum().getComponents() / 1_GeV);
      CORSIKA_LOG_INFO("Pythia::Decay: MinStep: gamma: {}", gamma);
      CORSIKA_LOG_INFO("Pythia::Decay: MinStep: tau: {} ", lifetime);

      return lifetime;
    } else
      return std::numeric_limits<double>::infinity() * 1_s;
  }

  template <typename TView>
  void Decay::doDecay(TView& view) {

    auto projectile = view.getProjectile();

    auto const& decayPoint = projectile.getPosition();
    auto const t0 = projectile.getTime();

    auto const& labMomentum = projectile.getMomentum();
    CoordinateSystemPtr const& labCS = labMomentum.getCoordinateSystem();

    // define target kinematics in lab frame
    // define boost to and from CoM frame
    // CoM frame definition in Pythia projectile: +z
    COMBoost const boost(labMomentum, projectile.getMass());
    auto const& rotatedCS = boost.getRotatedCS();

    count_++;

    // pythia stack
    Pythia8::Event& event = pythia_.event;
    event.reset();

    auto const particleId = projectile.getPID();

    // set particle unstable
    Decay::setUnstable(particleId);

    // input particle PDG
    auto const pdgCode = static_cast<int>(get_PDG(particleId));

    double constexpr px = 0;
    double constexpr py = 0;
    double constexpr pz = 0;
    double const en = projectile.getMass() / 1_GeV;
    double const m = en;

    // add particle to pythia stack
    event.append(pdgCode, 1, 0, 0, px, py, pz, en, m);

    if (!pythia_.next())
      throw std::runtime_error("Pythia::Decay: decay failed!");
    else
      CORSIKA_LOG_INFO("Pythia::Decay: particles after decay: {} ", event.size());

    if (print_listing_) {
      // list final state
      event.list();
    }

    // loop over final state
    for (int i = 0; i < event.size(); ++i)
      if (event[i].isFinal()) {
        auto const pyId = convert_from_PDG(static_cast<PDGCode>(event[i].id()));
        HEPEnergyType const Erest = event[i].e() * 1_GeV;
        MomentumVector const pRest(
            rotatedCS,
            {event[i].px() * 1_GeV, event[i].py() * 1_GeV, event[i].pz() * 1_GeV});
        FourVector const fourMomRest{Erest, pRest};
        auto const fourMomLab = boost.fromCoM(fourMomRest);

        CORSIKA_LOG_INFO("particle: id={} momentum={} energy={} ", pyId,
                         fourMomLab.getSpaceLikeComponents().getComponents(labCS) / 1_GeV,
                         fourMomLab.getTimeLikeComponent());

        view.addSecondary(std::make_tuple(pyId, fourMomLab.getTimeLikeComponent(),
                                          fourMomLab.getSpaceLikeComponents(), decayPoint,
                                          t0));
      }

    // set particle stable
    Decay::setStable(particleId);
  }
} // namespace corsika::pythia8
