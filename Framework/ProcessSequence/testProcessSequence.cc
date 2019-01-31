
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */


#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <array>
#include <iomanip>
#include <iostream>

#include <corsika/process/ProcessSequence.h>

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
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
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
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
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
  template <typename Particle, typename Stack>
  inline EProcessReturn DoInteraction(Particle&, Stack&) const {
    cout << "Process2::DoInteraction" << endl;
    return EProcessReturn::eOk;
  }
  template <typename Particle, typename Track>
  GrammageType GetInteractionLength(Particle&, Track&) const {
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
  template <typename Particle, typename Stack>
  inline EProcessReturn DoInteraction(Particle&, Stack&) const {
    cout << "Process3::DoInteraction" << endl;
    return EProcessReturn::eOk;
  }
  template <typename Particle, typename Track>
  GrammageType GetInteractionLength(Particle&, Track&) const {
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
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < nData; ++i) { d.p[i] /= 1.2; }
    return EProcessReturn::eOk;
  }
  // inline double MinStepLength(D& d) {
  template <typename Particle, typename Stack>
  EProcessReturn DoInteraction(Particle&, Stack&) const {
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
  template <typename Particle, typename Stack>
  EProcessReturn DoDecay(Particle&, Stack&) const {
    return EProcessReturn::eOk;
  }
};

struct DummyData {
  double p[nData] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};
struct DummyStack {};
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

    DummyStack s;
    DummyTrajectory t;

    auto sequence2 = cp1 << m2 << m3;
    GrammageType const tot = sequence2.GetTotalInteractionLength(s, t);
    InverseGrammageType const tot_inv = sequence2.GetTotalInverseInteractionLength(s, t);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;
  }

  SECTION("lifetime") {
    ContinuousProcess1 cp1(0);
    Process2 m2(1);
    Process3 m3(2);
    Decay1 d3(2);

    DummyStack s;

    auto sequence2 = cp1 << m2 << m3 << d3;
    TimeType const tot = sequence2.GetTotalLifetime(s);
    InverseTimeType const tot_inv = sequence2.GetTotalInverseLifetime(s);
    cout << "lambda_tot=" << tot << "; lambda_tot_inv=" << tot_inv << endl;
  }

  SECTION("sectionTwo") {

    ContinuousProcess1 cp1(0);
    ContinuousProcess2 cp2(3);
    Process2 m2(1);
    Process3 m3(2);

    auto sequence2 = cp1 << m2 << m3 << cp2;

    DummyData p;
    DummyStack s;
    DummyTrajectory t;

    cout << "-->init sequence2" << endl;
    globalCount = 0;
    sequence2.Init();
    cout << "-->docont" << endl;

    sequence2.DoContinuous(p, t, s);
    cout << "-->dodisc" << endl;
    // sequence2.DoInteraction(p, s);
    cout << "-->done" << endl;

    const int nLoop = 5;
    cout << "Running loop with n=" << nLoop << endl;
    for (int i = 0; i < nLoop; ++i) {
      sequence2.DoContinuous(p, t, s);
      // sequence2.DoInteraction(p, s);
    }
    for (int i = 0; i < nData; i++) { cout << "data[" << i << "]=" << p.p[i] << endl; }
    cout << "done" << endl;
  }
}
