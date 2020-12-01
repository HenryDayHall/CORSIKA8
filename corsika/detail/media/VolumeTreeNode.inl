/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/IVolume.hpp>
#include <corsika/media/IMediumModel.hpp>

namespace corsika {

  //! convenience function equivalent to Volume::Contains
  template <typename IModelProperties>
  bool VolumeTreeNode<IModelProperties>::Contains(corsika::Point const& p) const {
    return fGeoVolume->Contains(p);
  }

  template <typename IModelProperties>
  inline VolumeTreeNode<IModelProperties> const*
  VolumeTreeNode<IModelProperties>::Excludes(corsika::Point const& p) const {
    auto exclContainsIter =
        std::find_if(fExcludedNodes.cbegin(), fExcludedNodes.cend(),
                     [&](auto const& s) { return bool(s->Contains(p)); });

    return exclContainsIter != fExcludedNodes.cend() ? *exclContainsIter : nullptr;
  }

  /** returns a pointer to the sub-VolumeTreeNode which is "responsible" for the given
   * \class Point \p p, or nullptr iff \p p is not contained in this volume.
   */
  template <typename IModelProperties>
  VolumeTreeNode<IModelProperties> const*
  VolumeTreeNode<IModelProperties>::GetContainingNode(corsika::Point const& p) const {
    if (!Contains(p)) { return nullptr; }

    if (auto const childContainsIter =
            std::find_if(fChildNodes.cbegin(), fChildNodes.cend(),
                         [&](auto const& s) { return bool(s->Contains(p)); });
        childContainsIter == fChildNodes.cend()) // not contained in any of the children
    {
      if (auto const exclContainsIter = Excludes(p)) // contained in any excluded nodes
      {
        return exclContainsIter->GetContainingNode(p);
      } else {
        return this;
      }
    } else {
      return (*childContainsIter)->GetContainingNode(p);
    }
  }

  template <typename IModelProperties>
  template <typename TCallable, bool preorder>
  void VolumeTreeNode<IModelProperties>::walk(TCallable func) {
    if constexpr (preorder) { func(*this); }

    std::for_each(fChildNodes.begin(), fChildNodes.end(),
                  [&](auto& v) { v->walk(func); });

    if constexpr (!preorder) { func(*this); };
  }

  template <typename IModelProperties>
  void VolumeTreeNode<IModelProperties>::AddChild(typename VolumeTreeNode<IModelProperties>::VTNUPtr pChild) {
    pChild->fParentNode = this;
    fChildNodes.push_back(std::move(pChild));
    // It is a bad idea to return an iterator to the inserted element
    // because it might get invalidated when the vector needs to grow
    // later and the caller won't notice.
  }

  template <typename IModelProperties>
  void VolumeTreeNode<IModelProperties>::ExcludeOverlapWith(typename VolumeTreeNode<IModelProperties>::VTNUPtr const& pNode) {
    fExcludedNodes.push_back(pNode.get());
  }

  template <typename IModelProperties>
  template <class MediumType, typename... Args>
  auto VolumeTreeNode<IModelProperties>::CreateMedium(Args&&... args) {
    static_assert(std::is_base_of_v<IMediumModel, MediumType>,
                  "unusable type provided, needs to be derived from \"IMediumModel\"");

    return std::make_shared<MediumType>(std::forward<Args>(args)...);
  }

} // namespace corsika
