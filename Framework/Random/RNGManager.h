#ifndef _include_RNGManager_h_
#define _include_RNGManager_h_

#include <map>
#include <random>
#include <sstream>
#include <string>

/*!
 * With this class modules can register streams of random numbers.
 */

namespace corsika::random {

  using RNG = std::mt19937; //!< the actual RNG type that will be used

  class RNGManager {
    std::map<std::string, RNG> rngs;
    std::map<std::string, std::seed_seq> seeds;

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
    //void SetSeedSeq(std::string const& pStreamName, std::seed_seq& const pSeedSeq);
  };

} // namespace corsika::random
#endif
