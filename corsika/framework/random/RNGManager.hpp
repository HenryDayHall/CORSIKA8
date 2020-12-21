/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <map>
#include <cstdint>
#include <random>
#include <string>

#include <corsika/framework/utility/Singleton.hpp>
#include <corsika/framework/logging/Logging.hpp>

/*!
 * With this class modules can register streams of random numbers.
 */

namespace corsika {

  class RNGManager : public corsika::Singleton<RNGManager> {

    friend class corsika::Singleton<RNGManager>;

  public:
    typedef std::mt19937_64 prng_type;
    typedef std::uint64_t seed_type;
    typedef std::string string_type;
    typedef std::map<std::string, prng_type> streams_type;
    typedef std::map<std::string, std::seed_seq> seeds_type;

    RNGManager(RNGManager const&) = default;

    RNGManager& operator=(RNGManager const&) = delete;

    /*!
     * This function is to be called by a module requiring a random-number
     * stream during its initialization.
     *
     * \throws sth. when stream \a pModuleName is already registered
     */
    inline void registerRandomStream(string_type const& streamName);

    /*!
     * returns the pre-stored stream of given name \a pStreamName if
     * available
     */
    inline prng_type& getRandomStream(string_type const& streamName);

    /*!
     * Check whether a stream has been registered.
     */
    inline bool isRegistered(string_type const& streamName) const;

    /*!
     * dumps the names and states of all registered random-number streams
     * into a std::stringstream.
     */
    inline std::stringstream dumpState() const;

    /**
     * Set explicit seeds for all currently registered streams. The actual seed values
     * are incremented from \a vSeed.
     */
    inline void seedAll(seed_type seed);

    /**
     * Set seeds for all currently registered streams.
     */
    inline void seedAll(
        void); //!< seed all currently registered streams with "real" randomness

    /**
     * @fn const streams_type getRngs&()const
     * @brief Constant access to the streams.
     *
     * @pre
     * @post
     * @return RNGManager::streams_type
     */
    inline const streams_type& getRngs() const { return rngs_; }

    /**
     * @fn const seeds_type getSeeds&()const
     * @brief Constant access to the seeds.
     *
     * @pre
     * @post
     * @return RNGManager::seeds_type
     */
    inline const seeds_type& getSeeds() const { return seeds_; }

    /**
     * @fn streams_type Rngs()
     * @brief Non-constant access to the streams.
     *
     * @pre
     * @post
     * @return RNGManager::streams_type&
     */
    inline streams_type Rngs() { return rngs_; }

    /**
     * @fn seeds_type Seeds&()
     * @brief Non-constant access to seeds.
     *
     * @pre
     * @post
     * @return RNGManager::seeds_type&
     */
    inline seeds_type& Seeds() { return seeds_; }

  protected:
    RNGManager() = default;

  private:
    streams_type rngs_;
    seeds_type seeds_;
  };

  typedef typename RNGManager::prng_type default_prng_type;

} // namespace corsika

#include <corsika/detail/framework/random/RNGManager.inl>
