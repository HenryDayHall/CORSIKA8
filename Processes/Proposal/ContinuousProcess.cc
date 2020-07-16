#include <PROPOSAL/PROPOSAL.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/particle_cut/ParticleCut.h>
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

  bool ContinuousProcess::CanInteract(particles::Code pcode) const noexcept {
    if (std::find(tracked_particles.begin(), tracked_particles.end(), pcode) !=
        tracked_particles.end())
      return true;
    return false;
  }

  template <>
  ContinuousProcess::ContinuousProcess(SetupEnvironment const& _env,
                                       CORSIKA_ParticleCut& _cut)
      : cut(_cut)
      , fRNG(corsika::random::RNGManager::GetInstance().GetRandomStream("proposal")) {
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

  template <>
  HEPEnergyType ContinuousProcess::TotalEnergyLoss(SetupParticle const& vP,
                                                   GrammageType const& vDX) {
    auto calc = GetCalculator(vP);
    return vP.GetEnergy() - get<DISPLACEMENT>(calc->second)
                                    ->UpperLimitTrackIntegral(vP.GetEnergy() / 1_MeV,
                                                              vDX / 1_g * 1_cm * 1_cm) *
                                1_MeV;
  }

  template <>
  void ContinuousProcess::Scatter(SetupParticle& vP, HEPEnergyType const& loss,
                                  GrammageType const& grammage) {
    auto calc = GetCalculator(vP);
    auto d = vP.GetDirection().GetComponents();
    auto direction = PROPOSAL::Vector3D(d.GetX().magnitude(), d.GetY().magnitude(),
                                        d.GetZ().magnitude());
    auto E_f = vP.GetEnergy() - loss; // final energy
    std::uniform_real_distribution<double> distr(0., 1.);
    auto rnd = array<double, 4>();
    for (auto& it : rnd) it = distr(fRNG);
    auto [mean_dir, final_dir] =
        get<SCATTERING>(calc->second)
            ->Scatter(grammage / 1_g * square(1_cm), vP.GetEnergy() / 1_MeV, E_f / 1_MeV,
                      direction, rnd);
    auto vec = corsika::geometry::QuantityVector(
        final_dir.GetX() * E_f, final_dir.GetY() * E_f, final_dir.GetZ() * E_f);
    vP.SetMomentum(corsika::stack::MomentumVector(
        corsika::geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem(),
        vec));
  }

  template <>
  EProcessReturn ContinuousProcess::DoContinuous(SetupParticle& vP,
                                                 SetupTrack const& vT) {
    if (!CanInteract(vP.GetPID())) return process::EProcessReturn::eOk;
    auto dX = vP.GetNode()->GetModelProperties().IntegratedGrammage(vT, vT.GetLength());
    auto energy_loss = TotalEnergyLoss(vP, dX);
    if (vP.GetChargeNumber() != 0) Scatter(vP, energy_loss, dX);
    vP.SetEnergy(vP.GetEnergy() - energy_loss);
    if (vP.GetEnergy() < cut.GetECut()) return process::EProcessReturn::eParticleAbsorbed;
    vP.SetMomentum(vP.GetMomentum() * vP.GetEnergy() / vP.GetMomentum().GetNorm());
    return process::EProcessReturn::eOk;
  }

  template <>
  units::si::LengthType ContinuousProcess::MaxStepLength(SetupParticle const& vP,
                                                         SetupTrack const& vT) {
    auto energy_lim = 0.9 * vP.GetEnergy() / 1_MeV;
    auto calc = GetCalculator(vP);
    auto grammage = get<DISPLACEMENT>(calc->second)
                        ->SolveTrackIntegral(vP.GetEnergy() / 1_MeV, energy_lim) *
                    1_g / square(1_cm);
    return vP.GetNode()->GetModelProperties().ArclengthFromGrammage(vT, grammage) *
           1.0001;
  }

} // namespace corsika::process::proposal
