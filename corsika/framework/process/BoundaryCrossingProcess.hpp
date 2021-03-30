/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/media/Environment.hpp>

#include <type_traits>

namespace corsika {

  /** @ingroup Processes
      @{
  */
  
  
  template <typename TDerived>
  class BoundaryCrossingProcess : public BaseProcess<TDerived> {

    /*    static_assert(std::is_invocable_v<decltype(&TDerived<>::doBoundaryCrossing),
       TDerived&, passepartout>, "BoundaryCrossingProcess needs
       doBoundaryCrossing(TParticle, " "TParticle::node_type, TParticle::node_type)");*/

  public:
    /**
     * This method is called when a particle crosses the boundary between the nodes
     * \p from and \p to.
     */
    template <typename TParticle>
    ProcessReturn doBoundaryCrossing(TParticle&,
                                     typename TParticle::node_type const& from,
                                     typename TParticle::node_type const& to);
  };

  //! @}
  
} // namespace corsika
