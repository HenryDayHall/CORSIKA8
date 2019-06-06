/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

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

void corsika::random::RNGManager::SeedAll(uint64_t vSeed) {
  for (auto& entry : rngs) { entry.second.seed(vSeed++); }
}

void corsika::random::RNGManager::SeedAll() {
  std::random_device rd;

  for (auto& entry : rngs) {
    std::seed_seq sseq{rd(), rd(), rd(), rd(), rd(), rd()};
    entry.second.seed(sseq);
  }
}

/*
void corsika::random::RNGManager::SetSeedSeq(std::string const& pStreamName,
                                             std::seed_seq const& pSeedSeq) {
  seeds[pStreamName] = pSeedSeq;
}
*/
