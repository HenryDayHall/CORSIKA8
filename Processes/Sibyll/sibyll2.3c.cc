#include <corsika/process/sibyll/sibyll2.3c.h>

#include <corsika/random/RNGManager.h>

double s_rndm_(int&) {
  static corsika::random::RNG& rmng =
      corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  ;
  return rmng() / (double)rmng.max();
}
