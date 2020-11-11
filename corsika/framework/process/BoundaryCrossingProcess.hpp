n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/media/Environment.hpp>

namespace corsika {

  template <typename TDerived>
  class BoundaryCrossingProcess : public BaseProcess<TDerived> {
  public:
    /**
     * This method is called when a particle crosses the boundary between the nodes
     * \p from and \p to.
     */
    template <typename TParticle, typename TVolumeNode>
    ProcessReturn DoBoundaryCrossing(TParticle&, TVolumeNode const& from,
                                     TVolumeNode const& to);
  };

} // namespace corsika
