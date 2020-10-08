/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/cascade/testCascade.h>

#include <corsika/cascade/Cascade.h>

#include <corsika/process/ProcessSequence.h>
#include <corsika/process/NullModel.h>
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::units::si;
using namespace corsika::geometry;

#include <limits>
using namespace std;

/*
  The dummy env must support GetMagneticField(), and a density model

 */
auto MakeDummyEnv() {
  TestEnvironmentType env; // dummy environment
  auto& universe = *(env.GetUniverse());
  const geometry::CoordinateSystem& cs = env.GetCoordinateSystem();

  auto theMedium = TestEnvironmentType::CreateNode<Sphere>(
      Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
      1_m * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::UniformMagneticField<
      environment::HomogeneousMedium<TestEnvironmentInterface>>;

  theMedium->SetModelProperties<MyHomogeneousModel>(
      geometry::Vector(cs, 0_T, 0_T, 0_T), 1_g / (1_cm * 1_cm * 1_cm),
      environment::NuclearComposition(
          std::vector<particles::Code>{particles::Code::Proton}, std::vector<float>{1.}));

  universe.AddChild(std::move(theMedium));

  return env;
}

class ProcessSplit : public process::InteractionProcess<ProcessSplit> {

  int fCalls = 0;
  GrammageType fX0;

public:
  ProcessSplit(GrammageType const X0)
      : fX0(X0) {}

  template <typename Particle>
  corsika::units::si::GrammageType GetInteractionLength(Particle const&) const {
    return fX0;
  }

  template <typename TSecondaryView>
  corsika::process::EProcessReturn DoInteraction(TSecondaryView& view) {
    fCalls++;
    auto const projectile = view.GetProjectile();
    const HEPEnergyType E = projectile.GetEnergy();
    view.AddSecondary(std::make_tuple(projectile.GetPID(), E / 2,
                                      projectile.GetMomentum(), projectile.GetPosition(),
                                      projectile.GetTime()));
    view.AddSecondary(std::make_tuple(projectile.GetPID(), E / 2,
                                      projectile.GetMomentum(), projectile.GetPosition(),
                                      projectile.GetTime()));
    return EProcessReturn::eInteracted;
  }

  int GetCalls() const { return fCalls; }
};

class ProcessCut : public process::SecondariesProcess<ProcessCut> {

  int fCount = 0;
  int fCalls = 0;
  HEPEnergyType fEcrit;

public:
  ProcessCut(HEPEnergyType e)
      : fEcrit(e) {}

  template <typename TStack>
  EProcessReturn DoSecondaries(TStack& vS) {
    fCalls++;
    auto p = vS.begin();
    while (p != vS.end()) {
      HEPEnergyType E = p.GetEnergy();
      if (E < fEcrit) {
        p.Delete();
        fCount++;
      }
      ++p; // next particle
    }
    C8LOG_INFO(fmt::format("ProcessCut::DoSecondaries size={} count={}", vS.getEntries(),
                           fCount));
    return EProcessReturn::eOk;
  }

  int GetCount() const { return fCount; }
  int GetCalls() const { return fCalls; }
};

TEST_CASE("Cascade", "[Cascade]") {

  HEPEnergyType E0 = 100_GeV;

  random::RNGManager& rmng = random::RNGManager::GetInstance();
  rmng.RegisterRandomStream("cascade");

  auto env = MakeDummyEnv();
  auto const& rootCS = env.GetCoordinateSystem();
  tracking_line::TrackingLine tracking;

  stack_inspector::StackInspector<TestCascadeStack> stackInspect(1, true, E0);
  process::NullModel nullModel;

  const GrammageType X0 = 20_g / square(1_cm);
  const HEPEnergyType Ecrit = 85_MeV;
  ProcessSplit split(X0);
  ProcessCut cut(Ecrit);
  auto sequence = process::sequence(nullModel, stackInspect, split, cut);
  TestCascadeStack stack;
  stack.Clear();
  stack.AddParticle(std::make_tuple(
      particles::Code::Electron, E0,
      corsika::stack::MomentumVector(
          rootCS, {0_GeV, 0_GeV,
                   -sqrt(E0 * E0 - units::static_pow<2>(
                                       particles::GetMass(particles::Code::Electron)))}),
      Point(rootCS, {0_m, 0_m, 10_km}), 0_ns));

  cascade::Cascade<tracking_line::TrackingLine, decltype(sequence), TestCascadeStack,
                   TestCascadeStackView>
      EAS(env, tracking, sequence, stack);

  SECTION("full cascade") {
    EAS.Run();

    CHECK(cut.GetCount() == 2048);
    CHECK(cut.GetCalls() == 2047);
    CHECK(split.GetCalls() == 2047);
  }

  SECTION("forced interaction") {
    EAS.forceInteraction();
    CHECK(stack.getEntries() == 2);
    CHECK(split.GetCalls() == 1);
  }
}
