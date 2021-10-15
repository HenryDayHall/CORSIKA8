/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <testCascade.hpp>

#include <corsika/framework/core/Cascade.hpp>

#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/process/NullModel.hpp>
#include <corsika/modules/StackInspector.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <corsika/framework/utility/COMBoost.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <corsika/output/DummyOutputManager.hpp>

#include <SetupTestTrajectory.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

#include <limits>
using namespace std;

/**
 * testCascade implements an e.m. Heitler model with energy splitting
 * and a critical energy.
 *
 * It resembles one of the most simple cascades you can simulate with CORSIKA8.
 */

/*
  The dummy env (here) doesn't need to have any propoerties
 */

auto make_dummy_env() {
  TestEnvironmentType env; // dummy environment
  auto& universe = *(env.getUniverse());

  auto world = TestEnvironmentType::createNode<Sphere>(
      Point{env.getCoordinateSystem(), 0_m, 0_m, 0_m},
      1_km * std::numeric_limits<double>::infinity());

  NuclearComposition const composition({Code::Proton}, {1.});
  world->setModelProperties<TestEnvironmentInterface>(19.2_g / cube(1_cm), composition);

  universe.addChild(std::move(world));
  return env;
}

/**
 *
 * For the Heitler model we don't need particle transport.
 */
class DummyTracking {

public:
  template <typename TParticle>
  auto getTrack(TParticle const& particle) {
    VelocityVector const initialVelocity =
        particle.getMomentum() / particle.getEnergy() * constants::c;
    Line const theLine = Line(particle.getPosition(), initialVelocity);
    TimeType const tEnd = std::numeric_limits<TimeType::value_type>::infinity() * 1_s;
    return std::make_tuple(
        corsika::setup::testing::make_track<setup::Trajectory>(theLine, tEnd),
        // trajectory: just go ahead forever
        particle.getNode()); // next volume node
  }
  static std::string getName() { return "DummyTracking"; }
  static std::string getVersion() { return "1.0.0"; }
};

class ProcessSplit : public InteractionProcess<ProcessSplit> {

public:
  CrossSectionType getCrossSection(Code const, Code const, HEPEnergyType const) const {
    return 1_mb;
  }

  template <typename TView>
  void doInteraction(TView& view, COMBoost const&, Code, Code, HEPEnergyType) {
    ++calls_;
    auto vP = view.getProjectile();
    const HEPEnergyType Ekin = vP.getKineticEnergy();
    vP.addSecondary(std::make_tuple(vP.getPID(), Ekin / 2, vP.getMomentum().normalized(),
                                    vP.getPosition(), vP.getTime()));
    vP.addSecondary(std::make_tuple(vP.getPID(), Ekin / 2, vP.getMomentum().normalized(),
                                    vP.getPosition(), vP.getTime()));
  }

  int getCalls() const { return calls_; }

private:
  int calls_ = 0;
};

class ProcessCut : public SecondariesProcess<ProcessCut> {

public:
  ProcessCut(HEPEnergyType e)
      : fEcrit(e) {}

  template <typename TStack>
  void doSecondaries(TStack& vS) {
    calls_++;
    auto p = vS.begin();
    while (p != vS.end()) {
      HEPEnergyType E = p.getEnergy();
      if (E < fEcrit) {
        p.erase();
        count_++;
      }
      ++p; // next particle
    }
    CORSIKA_LOG_DEBUG("ProcessCut::doSecondaries size={} count={}", vS.getEntries(),
                      count_);
  }

  int getCount() const { return count_; }
  int getCalls() const { return calls_; }

private:
  int count_ = 0;
  int calls_ = 0;
  HEPEnergyType fEcrit;
};

TEST_CASE("Cascade", "[Cascade]") {

  logging::set_level(logging::level::info);

  HEPEnergyType E0 = 100_GeV;

  auto& rmng = RNGManager<>::getInstance();
  rmng.registerRandomStream("cascade");

  auto env = make_dummy_env();
  auto const& rootCS = env.getCoordinateSystem();

  StackInspector<TestCascadeStack> stackInspect(100, true, E0);
  NullModel nullModel;

  HEPEnergyType const Ecrit = 85_MeV;
  ProcessSplit split;
  ProcessCut cut(Ecrit);
  auto sequence = make_sequence(nullModel, stackInspect, split, cut);
  TestCascadeStack stack;
  stack.clear();
  stack.addParticle(std::make_tuple(
      Code::Electron,
      MomentumVector(rootCS, {0_GeV, 0_GeV,
                              -sqrt(E0 * E0 - static_pow<2>(get_mass(Code::Electron)))}),
      Point(rootCS, {0_m, 0_m, 10_km}), 0_ns));

  DummyTracking tracking;
  DummyOutputManager output;
  Cascade<DummyTracking, decltype(sequence), DummyOutputManager, TestCascadeStack> EAS(
      env, tracking, sequence, output, stack);

  SECTION("full cascade") {
    EAS.run();
    CHECK(cut.getCount() == 2048);
    CHECK(cut.getCalls() == 2047); // final particle is still on stack and not yet deleted
    CHECK(split.getCalls() == 2047);
  }

  SECTION("forced interaction") {
    EAS.setNodes();
    EAS.forceInteraction();
    CHECK(stack.getEntries() == 2);
    CHECK(stack.getSize() == 3);
    CHECK(split.getCalls() == 1);
  }
}
