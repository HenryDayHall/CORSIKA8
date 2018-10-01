#include <array>
#include <iomanip>
#include <iostream>

#include <corsika/process/ProcessSequence.h>

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
  inline EProcessReturn DoContinuous(D& , T& , S&) const {
    // for (int i=0; i<10; ++i) d.p[i] *= 2;
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
  double p[10];
};
struct DummyStack {};
struct DummyTrajectory {};

void modular() {

  Process1 m1;
  Process2 m2;
  Process3 m3;
  Process4 m4;

  const auto sequence = m1 + m2 + m3 + m4;

  DummyData p;
  DummyTrajectory t;
  DummyStack s;

  const int n = 100000000;
  for (int i = 0; i < n; ++i) { sequence.DoContinuous(p, t, s); }

  cout << " done (nothing...) " << endl;
}

int main() {
  modular();
  return 0;
}
