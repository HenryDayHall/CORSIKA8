/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <iterator>
#include <random>
#include <sstream>

namespace corsika {

  inline void RNGManager::registerRandomStream(string_type const& pStreamName) {
    prng_type rng;

    if (auto const& it = seeds_.find(pStreamName); it != seeds_.end()) {
      rng.seed(it->second);
    }

    rngs_[pStreamName] = std::move(rng);
  }

  inline RNGManager::prng_type& RNGManager::getRandomStream(
      string_type const& pStreamName) {
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

    for (auto& [streamName, rng] : rngs_) {
      std::seed_seq sseq{rd(), rd(), rd(), rd(), rd(), rd()}; // 6 really random values

      // for logging collect sseq input values in string
      std::stringstream ss;
      sseq.param(std::ostream_iterator<int>{ss, " "});
      CORSIKA_LOG_DEBUG("Random seed stream {} seed {}", streamName, ss.str());

      rng.seed(sseq);
    }
  }

} // namespace corsika
