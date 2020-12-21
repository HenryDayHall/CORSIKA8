/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <array>
#include <iomanip>
#include <iostream>

#include <corsika/framework/process/ProcessSequence.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

using namespace corsika;
using namespace std;

const int nData = 10;

class Process1 : public ContinuousProcess<Process1> {
public:
  Process1() {}
  template <typename D, typename T>
  ProcessReturn doContinuous(D& d, T&) const {
    for (int i = 0; i < nData; ++i) d.p[i] += 1;
    return ProcessReturn::Ok;
  }
};

class Process2 : public ContinuousProcess<Process2> {
public:
  Process2() {}

  template <typename D, typename T>
  inline ProcessReturn doContinuous(D& d, T&) const {
    for (int i = 0; i < nData; ++i) d.p[i] -= 0.1 * i;
    return ProcessReturn::Ok;
  }
};

class Process3 : public ContinuousProcess<Process3> {
public:
  Process3() {}

  template <typename D, typename T>
  inline ProcessReturn doContinuous(D&, T&) const {
    return ProcessReturn::Ok;
  }
};

class Process4 : public ContinuousProcess<Process4> {
public:
  Process4(const double v)
      : fV(v) {}
  template <typename D, typename T>
  inline ProcessReturn doContinuous(D& d, T&) const {
    for (int i = 0; i < nData; ++i) d.p[i] *= fV;
    return ProcessReturn::Ok;
  }

private:
  double fV;
};

struct DummyData {
  double p[nData] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};
struct DummyStack {
  void clear() {}
};
struct DummyTrajectory {};

void modular() {

  //              = 0
  Process1 m1;      // + 1.0
  Process2 m2;      // - (0.1*i)
  Process3 m3;      // * 1.0
  Process4 m4(1.5); // * 1.5

  auto sequence = make_sequence(m1, m2, m3, m4);

  DummyData particle;
  DummyTrajectory track;

  double check[nData] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  const int nEv = 10;
  for (int iEv = 0; iEv < nEv; ++iEv) {
    sequence.doContinuous(particle, track);
    for (int i = 0; i < nData; ++i) {
      check[i] += 1. - 0.1 * i;
      check[i] *= 1.5;
    }
  }

  for (int i = 0; i < nData; ++i) { assert(particle.p[i] == check[i]); }

  cout << " done (checking...) " << endl;
}

int main() {

  std::cout << "staticsequence_example" << std::endl;

  modular();
  return 0;
}
