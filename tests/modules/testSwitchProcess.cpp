/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/switch_process/SwitchProcess.h>
#include <corsika/stack/SecondaryView.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

#include <catch2/catch.hpp>

#include <algorithm>
#include <random>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units::si;

class TestStackData {

public:
  // these functions are needed for the Stack interface
  void Init() {}
  void Clear() { fData.clear(); }
  unsigned int GetSize() const { return fData.size(); }
  unsigned int GetCapacity() const { return fData.size(); }
  void Copy(int i1, int i2) { fData[i2] = fData[i1]; }
  void Swap(int i1, int i2) { std::swap(fData[i1], fData[i2]); }

  // custom data access function
  void SetData(unsigned int i, HEPEnergyType v) { fData[i] = v; }
  HEPEnergyType GetData(const unsigned int i) const { return fData[i]; }

  // these functions are also needed by the Stack interface
  void IncrementSize() { fData.resize(fData.size() + 1); }
  void DecrementSize() {
    if (fData.size() > 0) { fData.pop_back(); }
  }

  // custom private data section
private:
  std::vector<HEPEnergyType> fData;
};

/**
 * From static_cast of a StackIterator over entries in the
 * TestStackData class you get and object of type
 * TestParticleInterface defined here
 *
 * It provides Get/Set methods to read and write data to the "Data"
 * storage of TestStackData obtained via
 * "StackIteratorInterface::GetStackData()", given the index of the
 * iterator "StackIteratorInterface::GetIndex()"
 *
 */
template <typename StackIteratorInterface>
class TestParticleInterface
    : public corsika::stack::ParticleBase<StackIteratorInterface> {

public:
  using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;
  using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

  /*
     The SetParticleData methods are called for creating new entries
     on the stack. You can specifiy various parametric versions to
     perform this task:
  */

  // default version for particle-creation from input data
  void SetParticleData(const std::tuple<HEPEnergyType> v) { SetEnergy(std::get<0>(v)); }
  void SetParticleData(TestParticleInterface<StackIteratorInterface>& /*parent*/,
                       std::tuple<HEPEnergyType> v) {
    SetEnergy(std::get<0>(v));
  }

  // here are the fundamental methods for access to TestStackData data
  void SetEnergy(HEPEnergyType v) { GetStackData().SetData(GetIndex(), v); }
  HEPEnergyType GetEnergy() const { return GetStackData().GetData(GetIndex()); }
};

using SimpleStack = corsika::stack::Stack<TestStackData, TestParticleInterface>;

// see issue 161
#if defined(__clang__)
using StackTestView = corsika::stack::SecondaryView<TestStackData, TestParticleInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
using StackTestView = corsika::stack::MakeView<SimpleStack>::type;
#endif

auto constexpr kgMSq = 1_kg / (1_m * 1_m);

template <int N>
struct DummyProcess : InteractionProcess<DummyProcess<N>> {
  void Init() {}

  template <typename TParticle>
  corsika::units::si::GrammageType GetInteractionLength(TParticle const&) const {
    return N * kgMSq;
  }

  template <typename TSecondaries>
  corsika::process::EProcessReturn DoInteraction(TSecondaries& vSec) {
    // to figure out which process was selected in the end, we produce N
    // secondaries for DummyProcess<N>

    for (int i = 0; i < N; ++i) {
      vSec.AddSecondary(std::tuple<HEPEnergyType>{vSec.GetEnergy() / N});
    }

    return EProcessReturn::eOk;
  }
};

using DummyLowEnergyProcess = DummyProcess<1>;
using DummyHighEnergyProcess = DummyProcess<2>;
using DummyAdditionalProcess = DummyProcess<3>;

TEST_CASE("SwitchProcess from InteractionProcess") {
  DummyLowEnergyProcess low;
  DummyHighEnergyProcess high;
  DummyAdditionalProcess proc;

  switch_process::SwitchProcess switchProcess(low, high, 1_TeV);
  auto seq = switchProcess << proc;

  SimpleStack stack;

  SECTION("low energy") {
    stack.AddParticle(std::tuple<HEPEnergyType>{0.5_TeV});
    auto p = stack.GetNextParticle();

    // low energy process returns 1 kg/m²
    SECTION("interaction length") {
      REQUIRE(switchProcess.GetInteractionLength(p) / kgMSq == Approx(1));
      REQUIRE(seq.GetTotalInteractionLength(p) / kgMSq == Approx(3. / 4));
    }

    // low energy process creates 1 secondary
    //~ SECTION("SelectInteraction") {
    //~ typename SimpleStack::ParticleType theParticle =
    //~ stack.GetNextParticle(); // as in corsika::Cascade
    //~ StackTestView view(theParticle);
    //~ auto projectile = view.GetProjectile();

    //~ InverseGrammageType invLambda = 0 / kgMSq;
    //~ switchProcess.SelectInteraction(p, projectile, 0.01 / kgMSq, invLambda);

    //~ REQUIRE(view.GetSize() == 1);
    //~ }
  }

  SECTION("high energy") {
    stack.AddParticle(std::tuple<HEPEnergyType>{4_TeV});
    auto p = stack.GetNextParticle();

    // high energy process returns 2 kg/m²
    SECTION("interaction length") {
      REQUIRE(switchProcess.GetInteractionLength(p) / kgMSq == Approx(2));
      REQUIRE(seq.GetTotalInteractionLength(p) / kgMSq == Approx(6. / 5));
    }

    // high energy process creates 2 secondaries
    SECTION("SelectInteraction") {
      typename SimpleStack::ParticleType theParticle =
          stack.GetNextParticle(); // as in corsika::Cascade
      StackTestView view(theParticle);
      auto projectile = view.GetProjectile();

      InverseGrammageType invLambda = 0 / kgMSq;
      switchProcess.SelectInteraction(p, projectile, 0.01 / kgMSq, invLambda);

      REQUIRE(view.GetSize() == 2);
    }
  }
}

TEST_CASE("SwitchProcess from ProcessSequence") {
  DummyProcess<1> innerA;
  DummyProcess<2> innerB;
  DummyProcess<3> outer;
  DummyProcess<4> additional;

  auto seq = innerA << innerB;

  switch_process::SwitchProcess switchProcess(seq, outer, 1_TeV);
  auto completeSeq = switchProcess << additional;

  SimpleStack stack;

  SECTION("low energy") {
    stack.AddParticle(std::tuple<HEPEnergyType>{0.5_TeV});
    auto p = stack.GetNextParticle();

    SECTION("interaction length") {
      REQUIRE(switchProcess.GetInteractionLength(p) / kgMSq == Approx(2. / 3));
      REQUIRE(completeSeq.GetTotalInteractionLength(p) / kgMSq == Approx(4. / 7));
    }

    SECTION("SelectInteraction") {
      std::vector<int> numberOfSecondaries;

      for (int i = 0; i < 1000; ++i) {
        typename SimpleStack::ParticleType theParticle =
            stack.GetNextParticle(); // as in corsika::Cascade
        StackTestView view(theParticle);
        auto projectile = view.GetProjectile();

        double r = i / 1000.;
        InverseGrammageType invLambda = r * 7. / 4 / kgMSq;

        InverseGrammageType accumulator = 0 / kgMSq;
        completeSeq.SelectInteraction(p, projectile, invLambda, accumulator);

        numberOfSecondaries.push_back(view.GetSize());
      }

      auto const mean =
          std::accumulate(numberOfSecondaries.cbegin(), numberOfSecondaries.cend(), 0.) /
          numberOfSecondaries.size();
      REQUIRE(mean == Approx(12. / 7.).margin(0.01));
    }
  }

  SECTION("high energy") {
    stack.AddParticle(std::tuple<HEPEnergyType>{3.0_TeV});
    auto p = stack.GetNextParticle();

    SECTION("interaction length") {
      REQUIRE(switchProcess.GetInteractionLength(p) / kgMSq == Approx(3));
      REQUIRE(completeSeq.GetTotalInteractionLength(p) / kgMSq == Approx(12. / 7.));
    }

    SECTION("SelectInteraction") {
      std::vector<int> numberOfSecondaries;

      for (int i = 0; i < 1000; ++i) {
        typename SimpleStack::ParticleType theParticle =
            stack.GetNextParticle(); // as in corsika::Cascade
        StackTestView view(theParticle);
        auto projectile = view.GetProjectile();

        double r = i / 1000.;
        InverseGrammageType invLambda = r * 7. / 12. / kgMSq;

        InverseGrammageType accumulator = 0 / kgMSq;
        completeSeq.SelectInteraction(p, projectile, invLambda, accumulator);

        numberOfSecondaries.push_back(view.GetSize());
      }

      auto const mean =
          std::accumulate(numberOfSecondaries.cbegin(), numberOfSecondaries.cend(), 0.) /
          numberOfSecondaries.size();
      REQUIRE(mean == Approx(24. / 7.).margin(0.01));
    }
  }
}
