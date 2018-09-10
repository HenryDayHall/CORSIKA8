#include <corsika/stack/super_stupid/SuperStupidStack.h>
#include <corsika/particles/ParticleProperties.h>
#include <iomanip>
#include <iostream>

using namespace std;
// using namespace corsika::literals;
// using namespace corsika::io;

using namespace corsika::units;
using namespace corsika::stack;


void fill(corsika::stack::super_stupid::SuperStupidStack& s) {
  for (int i = 0; i < 11; ++i) {
    auto p = s.NewParticle();
    p.SetId(corsika::particles::Code::Electron);
    p.SetEnergy(1.5_GeV * i);
  }
}

void read(corsika::stack::super_stupid::SuperStupidStack& s) {
  cout << "found Stack with " << s.GetSize() << " particles. " << endl;
  EnergyType Etot;
  for (auto p : s) {
    Etot += p.GetEnergy();
    cout << "particle: " << p.GetId() << " with " << p.GetEnergy() / 1_GeV << " GeV"
         << endl;
  }
  cout << "Etot=" << Etot << " = " << Etot / 1_GeV << " GeV" << endl;
}

int main() {
  corsika::stack::super_stupid::SuperStupidStack s;
  fill(s);
  read(s);
  return 0;
}
