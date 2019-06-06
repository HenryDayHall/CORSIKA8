/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <array>
#include <iomanip>
#include <iostream>

#include <corsika/process/ProcessSequence.h>
#include <corsika/process/switch_process/SwitchProcess.h>

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::process;
using namespace std;

static const int nData = 10;

int globalCount = 0;

class ContinuousProcess1 : public ContinuousProcess<ContinuousProcess1> {
  int fV = 0;

public:
  ContinuousProcess1(const int v)
      : fV(v) {}
  void Init() {
    cout << "ContinuousProcess1::Init" << endl;
    assert(globalCount == fV);
    globalCount++;
  }
  template <typename D, typename T>
  inline EProcessReturn DoContinuous(D& d, T&) const {
    cout << "ContinuousProcess1::DoContinuous" << endl;
    for (int i = 0; i < nData; ++i) d.p[i] += 0.933;
    return EProcessReturn::eOk;
  }
};

class ContinuousProcess2 : public ContinuousProcess<ContinuousProcess2> {
  int fV = 0;

public:
  ContinuousProcess2(const int v)
      : fV(v) {}
  void Init() {
    cout << "ContinuousProcess2::Init" << endl;
    assert(globalCount == fV);
    globalCount++;
  }
  template <typename D, typename T>
  inline EProcessReturn DoContinuous(D& d, T&) const {
    cout << "ContinuousProcess2::DoContinuous" << endl;
    for (int i = 0; i < nData; ++i) d.p[i] += 0.933;
    return EProcessReturn::eOk;
  }
};

class Process1 : public InteractionProcess<Process1> {
public:
  Process1(const int v)
      : fV(v) {}
  void Init() {
    cout << "Process1::Init" << endl;
    assert(globalCount == fV);
    globalCount++;
  }
  template <typename D, typename S>
  inline EProcessReturn DoInteraction(D& d, S&) const {
    for (int i = 0; i < nData; ++i) d.p[i] += 1 + i;
    return EProcessReturn::eOk;
  }
  // private:
  int fV;
};

class Process2 : public InteractionProcess<Process2> {
  int fV = 0;

public:
  Process2(const int v)
      : fV(v) {}
  void Init() {
    cout << "Process2::Init" << endl;
    assert(globalCount == fV);
    globalCount++;
  }
  template <typename Particle>
  inline EProcessReturn DoInteraction(Particle&) const {
    cout << "Process2::DoInteraction" << endl;
    return EProcessReturn::eOk;
  }
  template <typename Particle>
  GrammageType GetInteractionLength(Particle&) const {
    cout << "Process2::GetInteractionLength" << endl;
    return 3_g / (1_cm * 1_cm);
  }
};

class Process3 : public InteractionProcess<Process3> {
  int fV = 0;

public:
  Process3(const int v)
      : fV(v) {}
  void Init() {
    cout << "Process3::Init" << endl;
    assert(globalCount == fV);
    globalCount++;
  }
  template <typename Particle>
  inline EProcessReturn DoInteraction(Particle&) const {
    cout << "Process3::DoInteraction" << endl;
    return EProcessReturn::eOk;
  }
  template <typename Particle>
  GrammageType GetInteractionLength(Particle&) const {
    cout << "Process3::GetInteractionLength" << endl;
    return 1_g / (1_cm * 1_cm);
  }
};

class Process4 : public BaseProcess<Process4> {
  int fV = 0;

public:
  Process4(const int v)
      : fV(v) {}
  void Init() {
    cout << "Process4::Init" << endl;
    assert(globalCount == fV);
    globalCount++;
  }
  template <typename D, typename T>
  inline EProcessReturn DoContinuous(D& d, T&) const {
    for (int i = 0; i < nData; ++i) { d.p[i] /= 1.2; }
    return EProcessReturn::eOk;
  }
  // inline double MinStepLength(D& d) {
  template <typename Particle>
  EProcessReturn DoInteraction(Particle&) const {
    return EProcessReturn::eOk;
  }
};

class Decay1 : public DecayProcess<Decay1> {
  int fV = 0;

public:
  Decay1(const int v)
      : fV(v) {}
  void Init() {
    cout << "Decay1::Init" << endl;
    assert(globalCount == fV);
    globalCount++;
  }
  template <typename Particle>
  TimeType GetLifetime(Particle&) const {
    return 1_s;
  }
  template <typename Particle>
  EProcessReturn DoDecay(Particle&) const {
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
  double p[nData] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};
struct DummyTrajectory {};

TEST_CASE("Process Sequence", "[Process Sequence]") {

  SECTION("Check init order") {
    Process1 m1(0);
    Process2 m2(1);
    Process3 m3(2);
    Process4 m4(3);

    auto sequence = m1 << m2 << m3 << m4;

    globalCount = 0;
    sequence.Init();
    // REQUIRE_NOTHROW( (sequence.Init()) );

    // const auto sequence_wrong = m3 + m2 + m1 + m4;
    // globalCount = 0;
    // sequence_wrong.Init();
    // REQUIRE_THROWS(sequence_wrong.Init());
  }

  SECTION("interaction length") {
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);

    DummyData particle;

    auto sequence2 = cp1 << m2 << m3;
    GrammageType const tot = sequence2.GetTotalInteractionLength(particle);
    InverseGrammageType const tot_inv =
        sequence2.GetTotalInverseInteractionLength(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;
  }

  SECTION("lifetime") {
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);
    Decay1 d3(2);

    DummyData particle;

    auto sequence2 = cp1 << m2 << m3 << d3;
    TimeType const tot = sequence2.GetTotalLifetime(particle);
    InverseTimeType const tot_inv = sequence2.GetTotalInverseLifetime(particle);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;
  }

  SECTION("sectionTwo") {

    ContinuousProcess1 cp1(0);
    ContinuousProcess2 cp2(3);
    Process2 m2(1);
    Process3 m3(2);

    auto sequence2 = cp1 << m2 << m3 << cp2;

    DummyData particle;
    DummyTrajectory track;

    cout << "-->init sequence2" << endl;
    globalCount = 0;
    sequence2.Init();
    cout << "-->docont" << endl;

    sequence2.DoContinuous(particle, track);
    cout << "-->dodisc" << endl;
    cout << "-->done" << endl;

    const int nLoop = 5;
    cout << "Running loop with n=" << nLoop << endl;
    for (int i = 0; i < nLoop; ++i) { sequence2.DoContinuous(particle, track); }
    for (int i = 0; i < nData; i++) {
      cout << "data[" << i << "]=" << particle.p[i] << endl;
    }
    cout << "done" << endl;
  }

  SECTION("StackProcess") {

    ContinuousProcess1 cp1(0);
    ContinuousProcess2 cp2(3);
    Process2 m2(1);
    Process3 m3(2);
    Stack1 s1(1);
    Stack1 s2(2);

    auto sequence = s1 << s2;

    DummyStack stack;

    const int nLoop = 20;
    for (int i = 0; i < nLoop; ++i) { sequence.DoStack(stack); }

    CHECK(s1.GetCount() == 20);
    CHECK(s2.GetCount() == 10);
  }
}

/*
  Note: there is a fine-grained dedicated test-suite for SwitchProcess
  in Processes/SwitchProcess/testSwtichProcess
 */
TEST_CASE("SwitchProcess") {
  Process1 p1(0);
  Process2 p2(0);
  switch_process::SwitchProcess s(p1, p2, 10_GeV);
  REQUIRE(is_switch_process_v<decltype(s)>);
}
