/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/BaseProcess.h>
#include <corsika/process/ProcessReturn.h> // for convenience
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {

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
    using _TDerived = TDerived;

    // here starts the interface part
    // -> enforce TDerived to implement DoContinuous...
    template <typename TParticle, typename TTrack>
    EProcessReturn DoContinuous(TParticle&, TTrack const&) const;

    // -> enforce TDerived to implement MaxStepLength...
    template <typename TParticle, typename TTrack>
    units::si::LengthType MaxStepLength(TParticle const& p, TTrack const& track) const;
  };

} // namespace corsika::process
