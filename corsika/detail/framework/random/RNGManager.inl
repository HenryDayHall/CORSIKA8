/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  inline void RNGManager::RegisterRandomStream(std::string const& pStreamName) {
    RNG rng;

    if (auto const& it = seeds.find(pStreamName); it != seeds.end()) {
      rng.seed(it->second);
    }

    rngs[pStreamName] = std::move(rng);
  }

  inline RNG& RNGManager::GetRandomStream(std::string const& pStreamName)  {
	  if (IsRegistered(pStreamName)) {
	    return rngs.at(pStreamName);
	  } else { // this stream name is not in the map
	    throw std::runtime_error("'" + pStreamName + "' is not a registered stream.");
	  }
	}


  inline bool RNGManager::IsRegistered(std::string const& pStreamName) const {
    return rngs.count(pStreamName) > 0;
  }


  inline std::stringstream RNGManager::dumpState() const {
    std::stringstream buffer;
    for (auto const& [streamName, rng] : rngs) {
      buffer << '"' << streamName << "\" = \"" << rng << '"' << std::endl;
    }

    return buffer;
  }

  inline void RNGManager::SeedAll(uint64_t vSeed) {
    for (auto& entry : rngs) { entry.second.seed(vSeed++); }
  }

  inline void RNGManager::SeedAll() {
    std::random_device rd;
    std::seed_seq sseq{rd(), rd(), rd(), rd(), rd(), rd()};
    for (auto& entry : rngs) {
      std::vector<std::uint32_t> seeds(1);
      sseq.generate(seeds.begin(), seeds.end());
      std::uint32_t seed = seeds[0];
      C8LOG_TRACE("Random seed stream {} seed {}", entry.first, seed);
      entry.second.seed(seed);
    }
  }


} // namespace corsika
