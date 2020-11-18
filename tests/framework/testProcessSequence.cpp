/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/ProcessSequence.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <iomanip>
#include <iostream>
#include <typeinfo>

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika;
using namespace std;

static const int nData = 10;

int globalCount = 0; // simple counter

int checkDecay = 0;    // use this as a bit field
int checkInteract = 0; // use this as a bit field
int checkSec = 0;      // use this as a bit field
int checkCont = 0;     // use this as a bit field

class ContinuousProcess1 : public ContinuousProcess<ContinuousProcess1> {
  [[maybe_unused]] int fV = 0;

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
  [[maybe_unused]] int fV = 0;

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
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.111;
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
    for (int i = 0; i < nData; ++i) d.data_[i] += 0.333;
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
private:
  [[maybe_unused]] int fV;
};

class Process2 : public InteractionProcess<Process2> {
  [[maybe_unused]] int fV = 0;

public:
  Process2(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline EProcessReturn DoInteraction(TView& v) const {
    checkInteract |= 2;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] /= 1.1;
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
  [[maybe_unused]] int fV = 0;

public:
  Process3(const int v)
      : fV(v) {
    cout << "globalCount: " << globalCount << ", fV: " << fV << std::endl;
    globalCount++;
  }

  template <typename TView>
  inline EProcessReturn DoInteraction(TView& v) const {
    checkInteract |= 4;
    for (int i = 0; i < nData; ++i) v.parent().data_[i] *= 1.01;
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
  [[maybe_unused]] int fV = 0;

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
  [[maybe_unused]] int fV = 0;

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

    auto sequence1 = process::sequence(m1, m2, m3, m4);
    CHECK(is_process_sequence_v<decltype(sequence1)> == true);
    CHECK(is_process_sequence_v<decltype(m2)> == false);
    CHECK(is_switch_process_sequence_v<decltype(sequence1)> == false);
    CHECK(is_switch_process_sequence_v<decltype(m2)> == false);

    auto sequence2 = process::sequence(m1, m2, m3);
    CHECK(is_process_sequence_v<decltype(sequence2)> == true);

    auto sequence3 = process::sequence(m4);
    CHECK(is_process_sequence_v<decltype(sequence3)> == true);
  }

  SECTION("interaction length") {
    globalCount = 0;
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);

    DummyData particle;

    auto sequence2 = sequence(cp1, m2, m3);
    GrammageType const tot = sequence2.GetInteractionLength(particle);
    InverseGrammageType const tot_inv = sequence2.GetInverseInteractionLength(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;

    CHECK(tot / 1_g * square(1_cm) == 12);
    CHECK(tot_inv * 1_g / square(1_cm) == 1. / 12);
    globalCount = 0;
  }

  SECTION("lifetime") {
    globalCount = 0;
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);
    Decay1 d3(3);

    DummyData particle;

    auto sequence2 = sequence(cp1, m2, m3, d3);
    TimeType const tot = sequence2.GetLifetime(particle);
    InverseTimeType const tot_inv = sequence2.GetInverseLifetime(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;

    CHECK(tot / 1_s == 1);
    CHECK(tot_inv * 1_s == 1.);
    globalCount = 0;
  }

  SECTION("sectionTwo") {
    globalCount = 0;
    ContinuousProcess1 cp1(0); // += 0.933
    ContinuousProcess2 cp2(1); // += 0.111
    Process2 m2(2);            //  /= 1.1
    Process3 m3(3);            //  *= 1.01

    auto sequence2 = sequence(cp1, m2, m3, cp2);

    DummyData particle;
    DummyTrajectory track;

    cout << "-->init sequence2" << endl;
    globalCount = 0;
    cout << "-->docont" << endl;

    // validation data
    double test_data[nData] = {0};

    const int nLoop = 5;
    cout << "Running loop with n=" << nLoop << endl;
    for (int iLoop = 0; iLoop < nLoop; ++iLoop) {
      for (int i = 0; i < nData; ++i) { test_data[i] += 0.933 + 0.111; }
      sequence2.DoContinuous(particle, track);
    }
    for (int i = 0; i < nData; i++) {
      cout << "data_[" << i << "]=" << particle.data_[i] << endl;
      CHECK(particle.data_[i] == Approx(test_data[i]).margin(1e-9));
    }
    cout << "done" << endl;
  }

  SECTION("StackProcess") {

    globalCount = 0;
    Stack1 s1(1);
    Stack1 s2(2);

    auto sequence = process::sequence(s1, s2);

    DummyStack stack;

    const int nLoop = 20;
    for (int i = 0; i < nLoop; ++i) { sequence.DoStack(stack); }

    CHECK(s1.GetCount() == 20);
    CHECK(s2.GetCount() == 10);

    ContinuousProcess2 cp2(1); // += 0.111
    Process2 m2(2);            //  /= 1.1
    auto sequence2 = process::sequence(cp2, m2);
    auto sequence3 = process::sequence(cp2, m2, s1);

    CHECK(is_process_sequence_v<decltype(sequence2)> == true);
    CHECK(is_process_sequence_v<decltype(sequence3)> == true);
    CHECK(contains_stack_process_v<decltype(sequence2)> == false);
    CHECK(contains_stack_process_v<decltype(sequence3)> == true);
  }
}

/*
  Note: there is a fine-grained dedicated test-suite for SwitchProcess
  in Processes/SwitchProcess/testSwtichProcess
 */
/*
TEST_CASE("SwitchProcess") {
  Process1 p1(0);
  Process2 p2(0);
  switch_process::SwitchProcess s(p1, p2, 10_GeV);
  REQUIRE(is_switch_process_v<decltype(s)>);
  }*/
