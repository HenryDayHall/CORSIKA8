
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_discreteprocess_h_
#define _include_corsika_discreteprocess_h_

#include <corsika/process/ProcessReturn.h> // for convenience

#include <iostream> // debug

namespace corsika::process {

  /**
     \class DiscreteProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type DiscreteProcess<T>

   */

  template <typename derived>
  struct DiscreteProcess {

    // DiscreteProcess() {
    // static_assert(mustProvide<derived>::mustProvide, "");
    //}

    derived& GetRef() { return static_cast<derived&>(*this); }
    const derived& GetRef() const { return static_cast<const derived&>(*this); }

    // here starts the interface part
    // -> enforce derived to implement DoDiscrete...
    template <typename Particle, typename Stack>
    inline EProcessReturn DoDiscrete(Particle&, Stack&) const; // {}

    //  private:
    template <typename D, typename T, typename S>
    inline EProcessReturn DoContinuous(D& d, T&, S&) const {
      std::cout << "yeah" << std::endl;
      return EProcessReturn::eOk;
    } // find out how to make this FINAL
    // void DoContinuous;
  };

} // namespace corsika::process

#endif
