/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/BaseProcess.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/framework/process/ProcessTraits.hpp>

namespace corsika {

  /**
     Processes with continuous effects along a particle Trajectory

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type ContinuousProcess<T>

   */

  template <typename TDerived>
  class ContinuousProcess : public BaseProcess<TDerived> {
  private:
  protected:
  public:
    // here starts the interface part
    // -> enforce TDerived to implement DoContinuous...
    template <typename TParticle, typename TTrack>
    ProcessReturn doContinuous(TParticle&, TTrack const&, bool const stepLimit) const;

    // -> enforce TDerived to implement MaxStepLength...
    template <typename TParticle, typename TTrack>
    LengthType getMaxStepLength(TParticle const& p, TTrack const& track) const;
  };

  /**
   * ProcessTraits specialization
   **/
  /*
    template <typename TProcess, int N>
    struct count_continuous<TProcess, N, 
                           typename std::enable_if_t<std::is_base_of_v<ContinuousProcess<typename std::decay_t<TProcess>>, 
                                                                       typename std::decay_t<TProcess>>>> {
      enum { count = N+1 };
    };
  */
} // namespace corsika
