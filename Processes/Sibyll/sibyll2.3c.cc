#include <corsika/process/sibyll/sibyll2.3c.h>

#include <corsika/random/RNGManager.h>
#include <random>

double s_rndm_(int&) {
  static corsika::random::RNG& rng =
      corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  
  std::uniform_real_distribution<double> dist;
  return dist(rng);
}
