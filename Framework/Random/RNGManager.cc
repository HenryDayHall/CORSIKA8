/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/random/RNGManager.h>
#include <sstream>

void corsika::random::RNGManager::RegisterRandomStream(std::string const& pStreamName) {
  corsika::random::RNG rng;

  if (auto const& it = seeds.find(pStreamName); it != seeds.end()) {
    rng.seed(it->second);
  }

  rngs[pStreamName] = std::move(rng);
}

corsika::random::RNG& corsika::random::RNGManager::GetRandomStream(
    std::string const& pStreamName) {
  if (IsRegistered(pStreamName)) {
    return rngs.at(pStreamName);
  } else { // this stream name is not in the map
    throw std::runtime_error(pStreamName + " is not a registered stream.");
  }
}

bool corsika::random::RNGManager::IsRegistered(std::string const& pStreamName) const {
  return rngs.count(pStreamName) > 0;
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
