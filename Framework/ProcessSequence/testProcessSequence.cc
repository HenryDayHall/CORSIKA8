#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <array>
#include <iomanip>
#include <iostream>

#include <corsika/process/ProcessSequence.h>

using namespace std;
using namespace corsika::process;

class Process1 : public BaseProcess<Process1> {
public:
  Process1() {}
  void Init() {} // cout << "Process1::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] += 1 + i;
    return EProcessReturn::eOk;
  }
};

class Process2 : public BaseProcess<Process2> {
public:
  Process2() {}
  void Init() {} // cout << "Process2::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] *= 0.7;
    return EProcessReturn::eOk;
  }
};

class Process3 : public BaseProcess<Process3> {
public:
  Process3() {}
  void Init() {} // cout << "Process3::Init" << endl; }
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] += 0.933;
    return EProcessReturn::eOk;
  }
};

class Process4 : public BaseProcess<Process4> {
public:
  Process4() {}
  void Init() {} // cout << "Process4::Init" << endl; }
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

    DummyData p;
    DummyTrajectory t;
    DummyStack s;

    sequence.Init();

    const int n = 100;
    INFO("Running loop with n=" << n);
    for (int i = 0; i < n; ++i) { sequence.DoContinuous(p, t, s); }

    for (int i = 0; i < 10; i++) { INFO("data[" << i << "]=" << p.p[i]); }
  }

  SECTION("sectionThree") {}
}
