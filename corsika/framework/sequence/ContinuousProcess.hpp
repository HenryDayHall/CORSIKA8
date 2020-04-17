/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/sequence/ProcessReturn.hpp> // for convenience

namespace corsika {

  /**
     \class ContinuousProcess

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
    EProcessReturn DoContinuous(TParticle&, TTrack const&) const;

    // -> enforce TDerived to implement MaxStepLength...
    template <typename TParticle, typename TTrack>
    units::si::LengthType MaxStepLength(TParticle const& p, TTrack const& track) const;
  };

  // overwrite the default trait class, to mark BaseProcess<T> as useful process
  template <class T>
  std::true_type is_process_impl(const ContinuousProcess<T>* impl);

} // namespace corsika


