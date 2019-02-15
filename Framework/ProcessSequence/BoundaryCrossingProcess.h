/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_processes_BoundaryCrossingProcess_h_
#define _include_corsika_processes_BoundaryCrossingProcess_h_

#include <corsika/environment/Environment.h>
#include <corsika/process/ProcessReturn.h>

namespace corsika::process {
  template <typename TDerived>
  struct BoundaryCrossingProcess {
    auto& GetRef() { return static_cast<TDerived&>(*this); }
    auto const& GetRef() const { return static_cast<const TDerived&>(*this); }

    /**
     * This method is called when a particle crosses the boundary between the nodes
     * \arg from and \arg to.
     */
    template <typename Particle>
    EProcessReturn DoBoundaryCrossing(Particle&, environment::BaseNodeType const& from,
                                      environment::BaseNodeType const& to);
  };

  template <class T>
  std::true_type is_process_impl(BoundaryCrossingProcess<T> const* impl);
} // namespace corsika::process

#endif
