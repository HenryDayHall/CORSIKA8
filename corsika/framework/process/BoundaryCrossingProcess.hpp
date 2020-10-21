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
  struct BoundaryCrossingProcess {
    auto& GetRef() { return static_cast<TDerived&>(*this); }
    auto const& GetRef() const { return static_cast<const TDerived&>(*this); }

  template <typename TDerived>
  class BoundaryCrossingProcess : public BaseProcess<TDerived> {
  private:
  protected:
  public:
    /**
     * This method is called when a particle crosses the boundary between the nodes
     * \p from and \p to.
     */
    template <typename TParticle, typename TVolumeNode>
    EProcessReturn DoBoundaryCrossing(TParticle&, TVolumeNode const& from,
                                      TVolumeNode const& to);
  };

  template <class T>
  std::true_type is_process_impl(BoundaryCrossingProcess<T> const* impl);

} // namespace corsika
