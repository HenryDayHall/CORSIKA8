
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <array>
#include <iomanip>
#include <iostream>

#include <corsika/process/ProcessSequence.h>

#include <corsika/setup/SetupTrajectory.h> // TODO: try to break this dependence later
using corsika::setup::Trajectory;
#include <corsika/units/PhysicalUnits.h> // dito
using namespace corsika::units::si;

using namespace std;
using namespace corsika::process;

class Process1 : public BaseProcess<Process1> {
public:
  Process1() {}
  template <typename D, typename T, typename S>
  EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i = 0; i < 10; ++i) d.p[i] += 1;
    return EProcessReturn::eOk;
  }
};

class Process2 : public BaseProcess<Process2> {
public:
  Process2() {}

  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& d, T&, S&) const {
    for (int i=0; i<10; ++i) d.p[i] -= 0.1*i;
    return EProcessReturn::eOk;
  }
};

class Process3 : public BaseProcess<Process3> {
public:
  // Process3(const int v) :fV(v) {}
  Process3() {}

  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& /*d*/, T& /*t*/, S& /*s*/) const {
    // for (int i=0; i<10; ++i) d.p[i] += fV;
    return EProcessReturn::eOk;
  }

private:
  // int fV;
};

class Process4 : public BaseProcess<Process4> {
public:
  // Process4(const int v) : fV(v) {}
  Process4() {}
  template <typename D, typename T, typename S>
  inline EProcessReturn DoContinuous(D& /*d*/, T& /*t*/, S& /*s*/) const {
    // for (int i=0; i<10; ++i) d.p[i] /= fV;
    return EProcessReturn::eOk;
  }

private:
  // int fV;
};

struct DummyData {
  double p[10] = {0,0,0,0,0,0,0,0,0,0};
};
struct DummyStack {};

void modular() {

  Process1 m1;
  Process2 m2;
  Process3 m3;
  Process4 m4;

  const auto sequence = m1 + m2 + m3 + m4;
  
  DummyData p;
  DummyStack s;
  Trajectory t;

  const int n = 1000;
  for (int i = 0; i < n; ++i) { sequence.DoContinuous(p, t, s); }
  
  for(int i=0; i<10; ++i) {
    //cout << p.p[i] << endl;
    //assert(p.p[i] == n-i*100);
  }
  
  cout << " done (nothing...) " << endl;
}

int main() {
  modular();
  return 0;
}
