
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/proposal/Interaction.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <tuple>

using Component_PROPOSAL = PROPOSAL::Components::Component;
namespace corsika::process::proposal {

  using namespace corsika::setup;
  using namespace corsika::environment;
  using namespace corsika::units::si;
  using SetupParticle = corsika::setup::Stack::StackIterator;

  template <>
  std::unordered_map<particles::Code, PROPOSAL::ParticleDef>
      Interaction<SetupEnvironment>::particles{
          {particles::Code::Gamma, PROPOSAL::GammaDef()},
          {particles::Code::Electron, PROPOSAL::EMinusDef()},
          {particles::Code::Positron, PROPOSAL::EPlusDef()},
          {particles::Code::MuMinus, PROPOSAL::MuMinusDef()},
          {particles::Code::MuPlus, PROPOSAL::MuPlusDef()},
          {particles::Code::TauPlus, PROPOSAL::TauPlusDef()},
          {particles::Code::TauMinus, PROPOSAL::TauMinusDef()},
      };

  template <>
  Interaction<SetupEnvironment>::Interaction(SetupEnvironment const& env,
                                             CORSIKA_ParticleCut const& e_cut)
      : fEnvironment(env)
      , cut(make_shared<const PROPOSAL::EnergyCutSettings>(e_cut.GetCutEnergy() / 1_GeV,
                                                           1, false)) {}

  template <>
  void Interaction<SetupEnvironment>::Init() {
    auto all_compositions = std::vector<NuclearComposition>();
    fEnvironment.GetUniverse()->walk([&](auto& vtn) {
      if (vtn.HasModelProperties())
        all_compositions.push_back(vtn.GetModelProperties().GetNuclearComposition());
    });
    for (auto& ncarg : all_compositions) {
      auto comp_vec = std::vector<Component_PROPOSAL>();
      auto frac_iter = ncarg.GetFractions().cbegin();
      for (auto& pcode : ncarg.GetComponents()) {
        comp_vec.emplace_back(GetName(pcode), GetNucleusZ(pcode), GetNucleusA(pcode),
                              *frac_iter);
        ++frac_iter;
      }
      media[&ncarg] = PROPOSAL::Medium(
          "Modified Air", 1., PROPOSAL::Air().GetI(), PROPOSAL::Air().GetC(),
          PROPOSAL::Air().GetA(), PROPOSAL::Air().GetM(), PROPOSAL::Air().GetX0(),
          PROPOSAL::Air().GetX1(), PROPOSAL::Air().GetD0(), 1.0, comp_vec);
    }
  }

  template <>
  template <>
  corsika::process::EProcessReturn Interaction<SetupEnvironment>::DoInteraction(
      SetupParticle& vP) {
    auto calc = GetCalculator(vP); // [CrossSections]
    std::uniform_real_distribution<double> distr(0., 1.);
    auto [type, comp_ptr, v] = std::get<INTERACTION>(calc->second)
                                   ->TypeInteraction(vP.GetEnergy() / 1_GeV, distr(fRNG));
    auto rnd = std::vector<double>();
    for (size_t i = 0;
         i < std::get<SECONDARIES>(calc->second).RequiredRandomNumbers(type); ++i)
      rnd.push_back(distr(fRNG));
    double primary_energy = vP.GetEnergy() / 1_GeV;
    auto point =
        PROPOSAL::Vector3D(vP.GetPosition().GetX() / 1_cm, vP.GetPosition().GetY() / 1_cm,
                           vP.GetPosition().GetZ() / 1_cm);
    auto p = vP.GetMomentum().GetComponents();
    auto direction = PROPOSAL::Vector3D(p[0] / 1_GeV, p[1] / 1_GeV, p[2] / 1_GeV);
    auto loss =
        make_tuple(static_cast<int>(type), point, direction, v * primary_energy, 0.);
    auto sec = std::get<SECONDARIES>(calc->second)
                   .CalculateSecondaries(primary_energy, loss, *comp_ptr, rnd);
    for (auto& s : sec) {
      auto energy = get<PROPOSAL::Loss::ENERGY>(s) * 1_MeV;
      auto vec = corsika::geometry::QuantityVector(
          std::get<PROPOSAL::Loss::DIRECTION>(s).GetX() * energy,
          std::get<PROPOSAL::Loss::DIRECTION>(s).GetY() * energy,
          std::get<PROPOSAL::Loss::DIRECTION>(s).GetZ() * energy);
      auto momentum = corsika::stack::MomentumVector(
          corsika::geometry::RootCoordinateSystem::GetInstance()
              .GetRootCoordinateSystem(),
          vec);
      particles::Code sec_code = corsika::particles::ConvertFromPDG(
          static_cast<particles::PDGCode>(get<PROPOSAL::Loss::TYPE>(s)));
      corsika::units::si::HEPEnergyType sec_energy = energy;
      stack::MomentumVector sec_momentum = momentum;
      geometry::Point sec_position = vP.GetPosition();
      corsika::units::si::TimeType sec_time = vP.GetTime();
      vP.AddSecondary(
          make_tuple(sec_code, sec_energy, sec_momentum, sec_position, sec_time));
    }
    return process::EProcessReturn::eOk;
  }

  template <>
  template <>
  corsika::units::si::GrammageType Interaction<SetupEnvironment>::GetInteractionLength(
      SetupParticle const& vP) {
    auto calc = GetCalculator(vP); // [CrossSections]
    std::uniform_real_distribution<double> distr(0., 1.);
    auto energy = get<INTERACTION>(calc->second)
                      ->EnergyInteraction(vP.GetEnergy() / 1_GeV, distr(fRNG));
    return get<DISPLACEMENT>(calc->second)
               ->SolveTrackIntegral(vP.GetEnergy() / 1_GeV, energy) *
           1_g / 1_cm / 1_cm;
  }
} // namespace corsika::process::proposal
