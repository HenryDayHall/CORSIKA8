
/**
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

using namespace std;
using namespace corsika::process;

class ContinuousProcess1 : public ContinuousProcess<ContinuousProcess1> {
public:
  ContinuousProcess1() {}
  void Init() { cout << "ContinuousProcess1::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    cout << "ContinuousProcess1::DoContinuous" << endl;
    for (int i = 0; i < 10; ++i) d.p[i] += 0.933;
    return EProcessReturn::eOk;
  }

  template <typename Particle, typename Stack>
  inline EProcessReturn DoDiscrete(Particle&, Stack&) const {
    cout << "ContinuousProcess1::DoDiscrete" << endl;
    return EProcessReturn::eOk;
  }
};

class ContinuousProcess2 : public ContinuousProcess<ContinuousProcess2> {
public:
  ContinuousProcess2() {}
  void Init() { cout << "ContinuousProcess2::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    cout << "ContinuousProcess2::DoContinuous" << endl;
    for (int i = 0; i < 20; ++i) d.p[i] += 0.933;
    return EProcessReturn::eOk;
  }

  template <typename Particle, typename Stack>
  inline EProcessReturn DoDiscrete(Particle&, Stack&) const {
    cout << "ContinuousProcess2::DoDiscrete" << endl;
    return EProcessReturn::eOk;
  }
};

class Process1 : public DiscreteProcess<Process1> {
public:
  Process1() {}
  void Init() { cout << "Process1::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] += 1 + i;
    return EProcessReturn::eOk;
  }
  template <typename Particle, typename Stack>
  inline EProcessReturn DoDiscrete(Particle&, Stack&) const {
    cout << "Process1::DoDiscrete" << endl;
    return EProcessReturn::eOk;
  }
};

class Process2 : public DiscreteProcess<Process2> {
public:
  Process2() {}
  void Init() { cout << "Process2::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] *= 0.7;
    return EProcessReturn::eOk;
  }
  template <typename Particle, typename Stack>
  inline EProcessReturn DoDiscrete(Particle&, Stack&) const {
    cout << "Process2::DoDiscrete" << endl;
    return EProcessReturn::eOk;
  }
};

class Process3 : public DiscreteProcess<Process3> {
public:
  Process3() {}
  void Init() { cout << "Process3::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] += 0.933;
    return EProcessReturn::eOk;
  }
  template <typename Particle, typename Stack>
  inline EProcessReturn DoDiscrete(Particle&, Stack&) const {
    cout << "Process3::DoDiscrete" << endl;
    return EProcessReturn::eOk;
  }
};

class Process4 : public BaseProcess<Process4> {
public:
  Process4() {}
  void Init() { cout << "Process4::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] /= 1.2;
    return EProcessReturn::eOk;
  }
  // inline double MinStepLength(D& d) {
  // void DoDiscrete(Particle& p, Stack& s) const {
};

struct DummyData {
  double p[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};
struct DummyStack {};
struct DummyTrajectory {};

TEST_CASE("Cascade", "[Cascade]") {

  SECTION("sectionTwo") {

    Process1 m1;
    Process2 m2;
    Process3 m3;
    Process4 m4;

    const auto sequence = m1 + m2 + m3 + m4;

    ContinuousProcess1 cp1;
    ContinuousProcess2 cp2;

    const auto sequence2 = cp1 + m2 + m3 + cp2;

    DummyData p;
    DummyTrajectory t;
    DummyStack s;

    cout << "-->init" << endl;
    sequence2.Init();
    cout << "-->docont" << endl;
    sequence2.DoContinuous(p, t, s);
    cout << "-->dodisc" << endl;
    sequence2.DoDiscrete(p, s);
    cout << "-->done" << endl;

    sequence.Init();

    const int n = 100;
    cout << "Running loop with n=" << n << endl;
    for (int i = 0; i < n; ++i) { sequence.DoContinuous(p, t, s); }

    for (int i = 0; i < 10; i++) { cout << "data[" << i << "]=" << p.p[i] << endl; }
  }

  SECTION("sectionThree") {}
}
