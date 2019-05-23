/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_RNGManager_h_
#define _include_RNGManager_h_

#include <corsika/utl/Singleton.h>

#include <map>
#include <random>
#include <sstream>
#include <string>

/*!
 * With this class modules can register streams of random numbers.
 */

namespace corsika::random {

  using RNG = std::mt19937; //!< the actual RNG type that will be used

  class RNGManager : public corsika::utl::Singleton<RNGManager> {

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
     * dumps the names and states of all registered random-number streams
     * into a std::stringstream.
     */
    std::stringstream dumpState() const;

    /**
     * set seed_seq of \a pStreamName to \a pSeedSeq
     */
    // void SetSeedSeq(std::string const& pStreamName, std::seed_seq& const pSeedSeq);

    /**
     * Set explicit seeds for all currently registered streams. The actual seed values
     * are incremented from \a vSeed.
     */
    void SeedAll(uint64_t vSeed);

    void SeedAll(); //!< seed all currently registered streams with "real" randomness
  };

} // namespace corsika::random
#endif
