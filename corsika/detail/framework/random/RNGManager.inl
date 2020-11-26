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

  inline void RNGManager::registerRandomStream(string_type const& pStreamName) {
    prng_type rng;

    if (auto const& it = seeds_.find(pStreamName); it != seeds_.end()) {
      rng.seed(it->second);
    }

    rngs_[pStreamName] = std::move(rng);
  }

  inline RNGManager::prng_type& RNGManager::getRandomStream(string_type const& pStreamName)  {
	  if (isRegistered(pStreamName)) {
	    return rngs_.at(pStreamName);
	  } else { // this stream name is not in the map
	    throw std::runtime_error("'" + pStreamName + "' is not a registered stream.");
	  }
	}


  inline bool RNGManager::isRegistered(string_type const& pStreamName) const {
    return rngs_.count(pStreamName) > 0;
  }


  inline std::stringstream RNGManager::dumpState() const {
    std::stringstream buffer;
    for (auto const& [streamName, rng] : rngs_) {
      buffer << '"' << streamName << "\" = \"" << rng << '"' << std::endl;
    }

    return buffer;
  }

  inline void RNGManager::seedAll(seed_type vSeed) {
    for (auto& entry : rngs_) { entry.second.seed(vSeed++); }
  }

  inline void RNGManager::seedAll(void) {
    std::random_device rd;
    std::seed_seq sseq{rd(), rd(), rd(), rd(), rd(), rd()};
    for (auto& entry : rngs_) {
      std::vector<std::uint32_t> seeds(1);
      sseq.generate(seeds.begin(), seeds.end());
      std::uint32_t seed = seeds[0];
      CORSIKA_LOG_TRACE("Random seed stream {} seed {}", entry.first, seed);
      entry.second.seed(seed);
    }
  }


} // namespace corsika
