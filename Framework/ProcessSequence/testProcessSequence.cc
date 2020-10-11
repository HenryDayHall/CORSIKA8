/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <array>
#include <iomanip>
#include <iostream>
#include <typeinfo>

#include <corsika/process/ProcessSequence.h>
#include <corsika/process/SwitchProcessSequence.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::process;
using namespace std;

static const int nData = 10;

int globalCount = 0; // simple counter

int checkDecay = 0;   // use this as a bit field
int checkInteract = 0; // use this as a bit field
int checkSec = 0;     // use this as a bit field
int checkCont = 0;    // use this as a bit field

class ContinuousProcess1 : public ContinuousProcess<ContinuousProcess1> {
  int fV = 0;

public:
  ContinuousProcess1(const int v)
      : fV(v) {

    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline EProcessReturn DoContinuous(D& d, T&) const {
    cout << "ContinuousProcess1::DoContinuous" << endl;
    checkCont |= 1;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.933;
    return EProcessReturn::eOk;
  }
};

class ContinuousProcess2 : public ContinuousProcess<ContinuousProcess2> {
  int fV = 0;

public:
  ContinuousProcess2(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline EProcessReturn DoContinuous(D& d, T&) const {
    cout << "ContinuousProcess2::DoContinuous" << endl;
    checkCont |= 2;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.933;
    return EProcessReturn::eOk;
  }
};

class ContinuousProcess3 : public ContinuousProcess<ContinuousProcess3> {
  int fV = 0;

public:
  ContinuousProcess3(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline EProcessReturn DoContinuous(D& d, T&) const {
    cout << "ContinuousProcess3::DoContinuous" << endl;
    checkCont |= 4;
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.933;
    return EProcessReturn::eOk;
  }
};

class Process1 : public InteractionProcess<Process1> {
public:
  Process1(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline EProcessReturn DoInteraction(TView& v) const {
    checkInteract |= 1;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] += 1 + i;
    return EProcessReturn::eOk;
  }

  template <typename TParticle>
  corsika::units::si::GrammageType GetInteractionLength(TParticle&) const {
    return 10_g / square(1_cm);
  }

private:
  int fV;
};

class Process2 : public InteractionProcess<Process2> {
  int fV = 0;

public:
  Process2(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline EProcessReturn DoInteraction(TView&) const {
    checkInteract |= 2;
    cout << "Process2::DoInteraction" << endl;
    return EProcessReturn::eOk;
  }
  template <typename Particle>
  GrammageType GetInteractionLength(Particle&) const {
    cout << "Process2::GetInteractionLength" << endl;
    return 20_g / (1_cm * 1_cm);
  }
};

class Process3 : public InteractionProcess<Process3> {
  int fV = 0;

public:
  Process3(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline EProcessReturn DoInteraction(TView&) const {
    checkInteract |= 4;
    cout << "Process3::DoInteraction" << endl;
    return EProcessReturn::eOk;
  }
  template <typename Particle>
  GrammageType GetInteractionLength(Particle&) const {
    cout << "Process3::GetInteractionLength" << endl;
    return 30_g / (1_cm * 1_cm);
  }
};

class Process4 : public BaseProcess<Process4> {
  int fV = 0;

public:
  Process4(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename D, typename T>
  inline EProcessReturn DoContinuous(D& d, T&) const {
    std::cout << "Base::DoContinuous" << std::endl;
    checkCont |= 8;
    for (int i = 0; i < nData; ++i) { d.data_[i] /= 1.2; }
    return EProcessReturn::eOk;
  }
  template <typename TView>
  EProcessReturn DoInteraction(TView&) const {
    checkInteract |= 8;
    return EProcessReturn::eOk;
  }
};

class Decay1 : public DecayProcess<Decay1> {

public:
  Decay1(const int) {
    cout << "Decay1()" << endl;
    globalCount++;
  }

  template <typename Particle>
  TimeType GetLifetime(Particle&) const {
    return 1_s;
  }
  template <typename TView>
  EProcessReturn DoDecay(TView&) const {
    checkDecay |= 1;
    return EProcessReturn::eOk;
  }
};

class Decay2 : public DecayProcess<Decay2> {

public:
  Decay2(const int) {
    cout << "Decay2()" << endl;
    globalCount++;
  }

  template <typename Particle>
  TimeType GetLifetime(Particle&) const {
    return 2_s;
  }
  template <typename TView>
  EProcessReturn DoDecay(TView&) const {
    checkDecay |= 2;
    return EProcessReturn::eOk;
  }
};

class Stack1 : public StackProcess<Stack1> {
  int fCount = 0;

public:
  Stack1(const int n)
      : StackProcess(n) {}
  template <typename TStack>
  EProcessReturn DoStack(TStack&) {
    fCount++;
    return EProcessReturn::eOk;
  }
  int GetCount() const { return fCount; }  
};

struct DummyStack {};
struct DummyData {
  double data_[nData] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};
struct DummyTrajectory {};

struct DummyView {
  DummyView(DummyData& p)
      : p_(p) {}
  DummyData& p_;
  DummyData& parent() { return p_; }
};

TEST_CASE("Process Sequence", "[Process Sequence]") {

  SECTION("Check construction") {
    globalCount = 0;
    Process1 m1(0);
    CHECK(globalCount == 1);
    Process2 m2(1);
    CHECK(globalCount == 2);
    Process3 m3(2);
    CHECK(globalCount == 3);
    Process4 m4(3);
    CHECK(globalCount == 4);

    auto sequence = m1 % m2 % m3 % m4;
    CHECK(is_process_sequence_v<decltype(sequence)> == true);
    CHECK(is_process_sequence_v<decltype(m2)> == false);
  }

  SECTION("interaction length") {
    globalCount = 0;
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);

    DummyData particle;

    auto sequence2 = cp1 % m2 % m3;
    GrammageType const tot = sequence2.GetInteractionLength(particle);
    InverseGrammageType const tot_inv = sequence2.GetInverseInteractionLength(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;
    globalCount = 0;
  }

  SECTION("lifetime") {
    globalCount = 0;
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);
    Decay1 d3(3);

    DummyData particle;

    auto sequence2 = cp1 % m2 % m3 % d3;
    TimeType const tot = sequence2.GetLifetime(particle);
    InverseTimeType const tot_inv = sequence2.GetInverseLifetime(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;

    globalCount = 0;
  }

  SECTION("sectionTwo") {
    globalCount = 0;
    ContinuousProcess1 cp1(0);
    ContinuousProcess2 cp2(1);
    Process2 m2(2);
    Process3 m3(3);

    auto sequence2 = cp1 % m2 % m3 % cp2;

    DummyData particle;
    DummyTrajectory track;

    cout << "-->init sequence2" << endl;
    globalCount = 0;
    cout << "-->docont" << endl;

    sequence2.DoContinuous(particle, track);
    cout << "-->dodisc" << endl;
    cout << "-->done" << endl;

    const int nLoop = 5;
    cout << "Running loop with n=" << nLoop << endl;
    for (int i = 0; i < nLoop; ++i) { sequence2.DoContinuous(particle, track); }
    for (int i = 0; i < nData; i++) {
      cout << "data_[" << i << "]=" << particle.data_[i] << endl;
    }
    cout << "done" << endl;
  }

  SECTION("StackProcess") {

    globalCount = 0;
    Stack1 s1(1);
    Stack1 s2(2);

    auto sequence = s1 % s2;

    DummyStack stack;

    const int nLoop = 20;
    for (int i = 0; i < nLoop; ++i) { sequence.DoStack(stack); }

    CHECK(s1.GetCount() == 20);
    CHECK(s2.GetCount() == 10);
  }
}

TEST_CASE("Switch Process Sequence", "[Process Sequence]") {

  SECTION("Check construction") {

    struct TestSelect {
      corsika::process::SwitchResult select(const DummyData& p) const {
        std::cout << "TestSelect data=" << p.data_[0] << std::endl;
        if (p.data_[0] > 0) return corsika::process::SwitchResult::First;
        return corsika::process::SwitchResult::Second;
      }
    };
    TestSelect select;

    auto sequence1 = Process1(0) % ContinuousProcess2(0) % Decay1(0);
    auto sequence2 = ContinuousProcess3(0) % Process2(0) % Decay2(0);

    auto sequence = ContinuousProcess1(0) % Process3(0) %
                    SwitchProcessSequence(sequence1, sequence2, select);

    auto sequence_alt =
        (ContinuousProcess1(0) % Process3(0)) %
        process::select(Process1(0) % ContinuousProcess2(0) % Decay1(0),
                        ContinuousProcess3(0) % Process2(0) % Decay2(0), select);

    // check that same process sequence can be build in different ways
    CHECK(typeid(sequence) == typeid(sequence_alt));
    CHECK(is_process_sequence_v<decltype(sequence)> == true);
    CHECK(is_process_sequence_v<decltype(
              SwitchProcessSequence(sequence1, sequence2, select))> == true);

    DummyData particle;
    DummyTrajectory track;
    DummyView view(particle);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = 100; // data positive
    sequence.DoContinuous(particle, track);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0b011);
    CHECK(checkSec == 0);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data negative
    sequence_alt.DoContinuous(particle, track);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0b101);
    CHECK(checkSec == 0);

    // 1/(30g/cm2) is Process3
    corsika::units::si::InverseGrammageType lambda_select = .9/30. * square(1_cm) / 1_g; 
    corsika::units::si::InverseTimeType time_select = 0.1 / second;

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = 100; // data positive
    sequence.SelectInteraction(view, lambda_select);
    sequence.SelectDecay(view, time_select);
    CHECK(checkInteract == 0b100); // this is Process3
    CHECK(checkDecay == 0b001); // this is Decay1
    CHECK(checkCont == 0);
    CHECK(checkSec == 0);
    lambda_select = 1.01/30. * square(1_cm) / 1_g; 
    checkInteract = 0;
    sequence.SelectInteraction(view, lambda_select);
    CHECK(checkInteract == 0b001); // this is Process1

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data negative
    sequence.SelectInteraction(view, lambda_select);
    sequence.SelectDecay(view, time_select);
    CHECK(checkInteract == 0b010); // this is Process2
    CHECK(checkDecay == 0b010); // this is Decay2
    CHECK(checkCont == 0);
    CHECK(checkSec == 0);

    checkDecay = 0;
    checkInteract = 0;
    checkSec = 0;
    checkCont = 0;
    particle.data_[0] = -100; // data negative
    sequence.DoSecondaries(view);
    Stack1 stack(0);
    sequence.DoStack(stack);
    CHECK(checkInteract == 0);
    CHECK(checkDecay == 0);
    CHECK(checkCont == 0);
    CHECK(checkSec == 0);
  }
}

