/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/IVolume.hpp>
#include <corsika/media/IMediumModel.hpp>

namespace corsika {

  //! convenience function equivalent to Volume::isInside
  template <typename IModelProperties>
  bool VolumeTreeNode<IModelProperties>::isInside(Point const& p) const {
    return geoVolume_->isInside(p);
  }

  template <typename IModelProperties>
  inline VolumeTreeNode<IModelProperties> const*
  VolumeTreeNode<IModelProperties>::isExcluded(Point const& p) const {
    auto exclContainsIter =
        std::find_if(excludedNodes_.cbegin(), excludedNodes_.cend(),
                     [&](auto const& s) { return bool(s->isInside(p)); });

    return exclContainsIter != excludedNodes_.cend() ? *exclContainsIter : nullptr;
  }

  /** returns a pointer to the sub-VolumeTreeNode which is "responsible" for the given
   * \class Point \p p, or nullptr iff \p p is not contained in this volume.
   */
  template <typename IModelProperties>
  VolumeTreeNode<IModelProperties> const*
  VolumeTreeNode<IModelProperties>::getContainingNode(Point const& p) const {
    if (!isInside(p)) { return nullptr; }

    if (auto const childContainsIter =
            std::find_if(childNodes_.cbegin(), childNodes_.cend(),
                         [&](auto const& s) { return bool(s->isInside(p)); });
        childContainsIter == childNodes_.cend()) // not contained in any of the children
    {
      if (auto const exclContainsIter = isExcluded(p)) // contained in any excluded nodes
      {
        return exclContainsIter->getContainingNode(p);
      } else {
        return this;
      }
    } else {
      return (*childContainsIter)->getContainingNode(p);
    }
  }

  template <typename IModelProperties>
  template <typename TCallable, bool preorder>
  void VolumeTreeNode<IModelProperties>::walk(TCallable func) {
    if constexpr (preorder) { func(*this); }

    std::for_each(childNodes_.begin(), childNodes_.end(),
                  [&](auto& v) { v->walk(func); });

    if constexpr (!preorder) { func(*this); };
  }

  template <typename IModelProperties>
  void VolumeTreeNode<IModelProperties>::addChild(
      typename VolumeTreeNode<IModelProperties>::VTNUPtr pChild) {
    pChild->parentNode_ = this;
    childNodes_.push_back(std::move(pChild));
    // It is a bad idea to return an iterator to the inserted element
    // because it might get invalidated when the vector needs to grow
    // later and the caller won't notice.
  }

  template <typename IModelProperties>
  void VolumeTreeNode<IModelProperties>::excludeOverlapWith(
      typename VolumeTreeNode<IModelProperties>::VTNUPtr const& pNode) {
    excludedNodes_.push_back(pNode.get());
  }

  /*
    template <typename IModelProperties>
    template <class MediumType, typename... Args>
    auto VolumeTreeNode<IModelProperties>::createMedium(Args&&... args) {
      static_assert(std::is_base_of_v<IMediumModel, MediumType>,
                    "unusable type provided, needs to be derived from \"IMediumModel\"");

      return std::make_shared<MediumType>(std::forward<Args>(args)...);
    }
  */
} // namespace corsika
