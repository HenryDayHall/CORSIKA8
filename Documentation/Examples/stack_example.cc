#include <fwk/StackOne.h>
//#include <corsika/StackOne.h>

#include <iomanip>
#include <iostream>

using namespace std;

//using namespace fwk;

void fill(stack::StackOne& s) {
  for (int i = 0; i < 11; ++i) {
    auto p = s.NewParticle();
    p.SetId(i);
    p.SetEnergy(1.5 * i);
  }
}

void read(stack::StackOne& s) {
  cout << "found Stack with " << s.GetSize() << " particles. " << endl;
  double Etot = 0;
  for (auto p : s) { Etot += p.GetEnergy(); }
  cout << "Etot=" << Etot << endl;
}

int main() {
  stack::StackOne s;
  fill(s);
  read(s);
  return 0;
}
