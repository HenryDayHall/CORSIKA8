#include <corsika/process/urqmd/UrQMD.h>
#include <corsika/random/RNGManager.h>
#include <random>

corsika::process::UrQMD::UrQMD::UrQMD() { iniurqmd_(); }

double ranf_(int*) {
  static corsika::random::RNG& rng =
      corsika::random::RNGManager::GetInstance().GetRandomStream("ranf");

  std::uniform_real_distribution<double> dist;
  return dist(rng);
}
