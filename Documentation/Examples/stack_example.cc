#include <stack/super_stupid/SuperStupidStack.h>
#include <fwk/Particles.h>

#include <iomanip>
#include <iostream>

using namespace std;
using namespace fwk::literals;
// using namespace fwk::io;

void fill(stack::super_stupid::SuperStupidStack& s) {
  for (int i = 0; i < 11; ++i) {
    auto p = s.NewParticle();
    p.SetId(fwk::particle::InternalParticleCode::Electron);
    p.SetEnergy(1.5_GeV * i);
  }
}

void read(stack::super_stupid::SuperStupidStack& s) {
  cout << "found Stack with " << s.GetSize() << " particles. " << endl;
  fwk::quantity<fwk::energy_d> Etot;
  for (auto p : s) {
    Etot += p.GetEnergy();
    cout << "particle: " << p.GetId() << " with " << p.GetEnergy()/1_GeV << " GeV" << endl;
  }
  cout << "Etot=" << Etot << " = " << Etot / 1_GeV << " GeV" << endl;
}

int main() {
  stack::super_stupid::SuperStupidStack s;
  fill(s);
  read(s);
  return 0;
}
