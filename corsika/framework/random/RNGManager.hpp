/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/utility/Singleton.hpp>
#include <corsika/framework/logging/Logging.h>
#include <map>
#include <random>
#include <string>

/*!
 * With this class modules can register streams of random numbers.
 */

namespace corsika {

  // FIXME: This while facility needs to re-designed.
  // It is not parallel friendly neither polymorphic
  // and the streaming management is prone to produce
  // huge correlation between the streams

  using RNG = std::mt19937; //!< the actual RNG type that will be used

  class RNGManager : public corsika::Singleton<RNGManager> {

    friend class corsika::Singleton<RNGManager>;

    std::map<std::string, RNG> rngs_;
    std::map<std::string, std::seed_seq> seeds_;

  protected:
    RNGManager() {} // why ?

  public:
    /*!
     * This function is to be called by a module requiring a random-number
     * stream during its initialization.
     *
     * \throws sth. when stream \a pModuleName is already registered
     */
    void registerRandomStream(std::string const& pStreamName);

    /*!
     * returns the pre-stored stream of given name \a pStreamName if
     * available
     */
    RNG& getRandomStream(std::string const& pStreamName);

    /*!
     * Check whether a stream has been registered.
     */
    bool isRegistered(std::string const& pStreamName) const;

    /*!
     * dumps the names and states of all registered random-number streams
     * into a std::stringstream.
     */
    std::stringstream dumpState() const;

    /**
     * Set explicit seeds for all currently registered streams. The actual seed values
     * are incremented from \a vSeed.
     */
    void seedAll(uint64_t vSeed);

    void seedAll(void); //!< seed all currently registered streams with "real" randomness
  };

} // namespace corsika

#include <corsika/detail/framework/random/RNGManager.inl>
