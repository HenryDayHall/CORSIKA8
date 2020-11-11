/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <testCascade.h>

#include <corsika/framework/core/Cascade.hpp>

#include <corsika/framework/sequence/ProcessSequence.hpp>
#include <corsika/framework/sequence/NullModel.hpp>
#include <corsika/modules/StackInspector.hpp>
#include <corsika/modules/TrackingLine.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika;
using namespace corsika::units;
using namespace corsika::units::si;
using namespace corsika;

#include <limits>
using namespace std;

/**
 * testCascade implements an e.m. Heitler model with energy splitting
 * and a critical energy.
 *
 * It resembles one of the most simple cascades you can simulate with CORSIKA8.
 **/

/*
  The dummy env (here) doesn't need to have any propoerties
 */
auto MakeDummyEnv() {
  TestEnvironmentType env; // dummy environment
  auto& universe = *(env.GetUniverse());

  auto world = TestEnvironmentType::CreateNode<Sphere>(
      Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
      1_m * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_g / (1_cm * 1_cm * 1_cm),
      environment::NuclearComposition(std::vector<Code>{Code::Proton},
                                      std::vector<float>{1.}));

  universe.AddChild(std::move(world));

  return env;
}

/**
 * \class DummyTracking
 *
 * For the Heitler model we don't need particle transport.
 **/
class DummyTracking {

public:
  template <typename TParticle>
  auto GetTrack(TParticle const& particle) {
    using namespace corsika::units::si;
    using namespace corsika::geometry;
    geometry::Vector<SpeedType::dimension_type> const initialVelocity =
        particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;
    return std::make_tuple(
        geometry::LineTrajectory(
            geometry::Line(particle.GetPosition(), initialVelocity),
            std::numeric_limits<TimeType::value_type>::infinity() * 1_s), // trajectory,
                                                                          // just
                                                                          // go
                                                                          // ahead
                                                                          // forever
        particle.GetNode()); // next volume node
  }
};

class ProcessSplit : public process::InteractionProcess<ProcessSplit> {

  int fCalls = 0;

public:
  template <typename Particle>
  corsika::units::si::GrammageType GetInteractionLength(Particle const&) const {
    return 0_g / square(1_cm);
  }

  template <typename TProjectile>
  corsika::EProcessReturn DoInteraction(TProjectile& vP) {
    fCalls++;
    const HEPEnergyType E = vP.GetEnergy();
    vP.AddSecondary(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                               geometry::Point, units::si::TimeType>{
        vP.GetPID(), E / 2, vP.GetMomentum(), vP.GetPosition(), vP.GetTime()});
    vP.AddSecondary(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                               geometry::Point, units::si::TimeType>{
        vP.GetPID(), E / 2, vP.GetMomentum(), vP.GetPosition(), vP.GetTime()});
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

  logging::SetLevel(logging::level::trace);

  HEPEnergyType E0 = 100_GeV;

  random::RNGManager& rmng = random::RNGManager::getInstance();
  rmng.registerRandomStream("cascade");

  auto env = MakeDummyEnv();
  auto const& rootCS = env.GetCoordinateSystem();

  stack_inspector::StackInspector<TestCascadeStack> stackInspect(1, true, E0);
  process::NullModel nullModel;

  const HEPEnergyType Ecrit = 85_MeV;
  ProcessSplit split;
  ProcessCut cut(Ecrit);
  auto sequence = process::sequence(nullModel, stackInspect, split, cut);
  TestCascadeStack stack;
  stack.Clear();
  stack.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                               geometry::Point, units::si::TimeType>{
      Code::Electron, E0, corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}),
      Point(rootCS, {0_m, 0_m, 10_km}), 0_ns});

  cascade::Cascade<tracking_line::TrackingLine, decltype(sequence), TestCascadeStack,
                   TestCascadeStackView>
      EAS(env, tracking, sequence, stack);

  SECTION("full cascade") {
    EAS.Run();

    CHECK(cut.GetCount() == 2048);
    CHECK(cut.GetCalls() == 2047); // final particle is still on stack and not yet deleted
    CHECK(split.GetCalls() == 2047);
  }

  SECTION("forced interaction") {
    EAS.forceInteraction();
    CHECK(stack.getEntries() == 2);
    CHECK(split.GetCalls() == 1);
  }
}
