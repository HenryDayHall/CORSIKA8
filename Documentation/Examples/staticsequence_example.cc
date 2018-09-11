#include <array>
#include <iomanip>
#include <iostream>

#include <corsika/process/ProcessSequence.h>

using namespace std;
using namespace corsika::process;

class Process1 : public corsika::process::BaseProcess<Process1> {
public:
  Process1() {}
  template <typename D>
  void DoContinuous(D& d) const {
    for (int i = 0; i < 10; ++i) d.p[i] += 1;
  }
};

class Process2 : public corsika::process::BaseProcess<Process2> {
public:
  Process2() {}

  template <typename D>
  inline void DoContinuous(D& d) const {
    // for (int i=0; i<10; ++i) d.p[i] *= 2;
  }
};

class Process3 : public BaseProcess<Process3> {
public:
  // Process3(const int v) :fV(v) {}
  Process3() {}

  template <typename D>
  inline void DoContinuous(D& d) const {
    // for (int i=0; i<10; ++i) d.p[i] += fV;
  }

private:
  // int fV;
};

class Process4 : public BaseProcess<Process4> {
public:
  // Process4(const int v) : fV(v) {}
  Process4() {}
  template <typename D>
  inline void DoContinuous(D& d) const {
    // for (int i=0; i<10; ++i) d.p[i] /= fV;
  }

private:
  // int fV;
};

class Data {
public:
  std::array<double, 10> p{{0.}};
};

void modular() {
  Data d0;

  Process1 m1;
  Process2 m2;
  Process3 m3;
  Process4 m4;

  const auto sequence = m1 + m2 + m3 + m4;

  const int n = 100000000;
  for (int i = 0; i < n; ++i) { sequence.DoContinuous(d0); }

  double s = 0;
  for (int i = 0; i < 10; ++i) { s += d0.p[i]; }

  cout << scientific << " v=" << s << " n=" << n << endl;
}

int main() {
  modular();
  return 0;
}
