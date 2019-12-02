#include <PROPOSAL/PROPOSAL.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/proposal/ContinuousProcess.h>
#include <corsika/process/proposal/Interaction.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

using SetupParticle = corsika::setup::Stack::ParticleType;
using SetupTrack = corsika::setup::Trajectory;

namespace corsika::process::proposal {
  using namespace corsika::setup;
  using namespace corsika::environment;
  using namespace corsika::units::si;

  unordered_map<particles::Code, PROPOSAL::ParticleDef> ContinuousProcess::particles{
      {particles::Code::Gamma, PROPOSAL::GammaDef()},
      {particles::Code::Electron, PROPOSAL::EMinusDef()},
      {particles::Code::Positron, PROPOSAL::EPlusDef()},
      {particles::Code::MuMinus, PROPOSAL::MuMinusDef()},
      {particles::Code::MuPlus, PROPOSAL::MuPlusDef()},
      {particles::Code::TauPlus, PROPOSAL::TauPlusDef()},
      {particles::Code::TauMinus, PROPOSAL::TauMinusDef()},
  };

  bool ContinuousProcess::CanInteract(particles::Code pcode) const noexcept {
    auto search = particles.find(pcode);
    if (search != particles.end()) return true;
    return false;
  }

  template <>
  ContinuousProcess::ContinuousProcess(SetupEnvironment const& _env,
                                       CORSIKA_ParticleCut const& _cut)
      : cut(make_shared<const PROPOSAL::EnergyCutSettings>(_cut.GetECut() / 1_MeV, 1,
                                                           false)) {
    auto all_compositions = std::vector<const NuclearComposition*>();
    _env.GetUniverse()->walk([&](auto& vtn) {
      if (vtn.HasModelProperties())
        all_compositions.push_back(&vtn.GetModelProperties().GetNuclearComposition());
    });
    for (auto& ncarg : all_compositions) {
      auto comp_vec = std::vector<PROPOSAL::Components::Component>();
      auto frac_iter = ncarg->GetFractions().cbegin();
      for (auto& pcode : ncarg->GetComponents()) {
        comp_vec.emplace_back(GetName(pcode), GetNucleusZ(pcode), GetNucleusA(pcode),
                              *frac_iter);
        ++frac_iter;
      }
      media[ncarg] = PROPOSAL::Medium(
          "Modified Air", 1., PROPOSAL::Air().GetI(), PROPOSAL::Air().GetC(),
          PROPOSAL::Air().GetA(), PROPOSAL::Air().GetM(), PROPOSAL::Air().GetX0(),
          PROPOSAL::Air().GetX1(), PROPOSAL::Air().GetD0(), 1.0, comp_vec);
    }
  }

  HEPEnergyType ContinuousProcess::TotalEnergyLoss(SetupParticle const& vP,
                                                   GrammageType const vDX) {
    auto calc_ptr = GetCalculator(vP);
    auto upper_energy = calc_ptr->second->UpperLimitTrackIntegral(
        vP.GetEnergy() / 1_MeV, vDX / 1_g * 1_cm * 1_cm);
    std::cout << "upper_energy: " << upper_energy << "MeV" << std::endl;
    return upper_energy * 1_MeV;
  }

   void ContinuousProcess::Init() {}
  template <>
  EProcessReturn ContinuousProcess::DoContinuous(SetupParticle& vP, SetupTrack const& vT) {
    if (vP.GetChargeNumber() == 0) return process::EProcessReturn::eOk;
    std::cout << "DoContinuous..." << std::endl;
    GrammageType const dX =
        vP.GetNode()->GetModelProperties().IntegratedGrammage(vT, vT.GetLength());
    HEPEnergyType dE = TotalEnergyLoss(vP, dX);
    auto E = vP.GetEnergy();
    const auto Ekin = E - vP.GetMass();
    auto Enew = E + dE;
    auto status = process::EProcessReturn::eOk;
    if (-dE > Ekin) {
      dE = -Ekin;
      Enew = vP.GetMass();
      status = process::EProcessReturn::eParticleAbsorbed;
    }
    vP.SetEnergy(Enew);
    auto pnew = vP.GetMomentum();
    vP.SetMomentum(pnew * Enew / pnew.GetNorm());
    return status;
  }

  template <>
  units::si::LengthType ContinuousProcess::MaxStepLength(SetupParticle const& vP,
                                                         SetupTrack const& vT) {
    auto constexpr dX = 1_g / square(1_cm);
    auto const dE = -TotalEnergyLoss(vP, dX); // dE > 0
    auto const maxLoss = 0.01 * vP.GetEnergy();
    auto const maxGrammage = maxLoss / dE * dX;

    return vP.GetNode()->GetModelProperties().ArclengthFromGrammage(vT,
                                                                    maxGrammage) *
           1.0001; // to make sure particle gets absorbed when DoContinuous() is called
  }

} // namespace corsika::process::proposal
