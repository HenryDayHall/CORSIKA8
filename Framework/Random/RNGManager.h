/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/utl/Singleton.h>

#include <map>
#include <random>
#include <string>

/*!
 * With this class modules can register streams of random numbers.
 */

namespace corsika::random {

  using RNG = std::mt19937; //!< the actual RNG type that will be used

  /*!
   * Manage random number generators.
   */
  class RNGManager final : public corsika::utl::Singleton<RNGManager> {

    friend class corsika::utl::Singleton<RNGManager>;

    std::map<std::string, RNG> rngs;
    std::map<std::string, std::seed_seq> seeds;

  protected:
    RNGManager() {}

  public:
    /*!
     * This function is to be called by a module requiring a random-number
     * stream during its initialization.
     *
     * \throws sth. when stream \a pModuleName is already registered
     */
    void RegisterRandomStream(std::string const& pStreamName);

    /*!
     * returns the pre-stored stream of given name \a pStreamName if
     * available
     */
    RNG& GetRandomStream(std::string const& pStreamName);

    /*!
     * Check whether a stream has been registered.
     */
    bool IsRegistered(std::string const& pStreamName) const;

    /*!
     * dumps the names and states of all registered random-number streams
     * into a std::stringstream.
     */
    std::stringstream dumpState() const;

    /**
     * Set explicit seeds for all currently registered streams. The actual seed values
     * are incremented from \a vSeed.
     */
    void SeedAll(uint64_t vSeed);

    void SeedAll(); //!< seed all currently registered streams with "real" randomness
  };

} // namespace corsika::random
