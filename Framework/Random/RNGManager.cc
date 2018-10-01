#include <corsika/random/RNGManager.h>

void corsika::random::RNGManager::RegisterRandomStream(std::string const& pStreamName) {
  corsika::random::RNG rng;

  if (auto const& it = seeds.find(pStreamName); it != seeds.end()) {
    rng.seed(it->second);
  }

  rngs[pStreamName] = std::move(rng);
}

corsika::random::RNG& corsika::random::RNGManager::GetRandomStream(
    std::string const& pStreamName) {
  return rngs.at(pStreamName);
}

std::stringstream corsika::random::RNGManager::dumpState() const {
  std::stringstream buffer;
  for (auto const& [streamName, rng] : rngs) {
    buffer << '"' << streamName << "\" = \"" << rng << '"' << std::endl;
  }

  return buffer;
}

void corsika::random::RNGManager::SetSeedSeq(std::string const& pStreamName,
                                             std::seed_seq const& pSeedSeq) {
  seeds[pStreamName] = pSeedSeq;
}
