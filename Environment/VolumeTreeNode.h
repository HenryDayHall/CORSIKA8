/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_VolumeTreeNode_H
#define _include_VolumeTreeNode_H

#include <corsika/environment/IMediumModel.h>
#include <corsika/geometry/Volume.h>
#include <memory>
#include <vector>

namespace corsika::environment {

  class Empty {}; //<! intended for usage as default template argument

  template <typename TModelProperties = Empty>
  class VolumeTreeNode {
  public:
    using IModelProperties = TModelProperties;
    using VTNUPtr = std::unique_ptr<VolumeTreeNode<IModelProperties>>;
    using IMPSharedPtr = std::shared_ptr<IModelProperties>;
    using VolUPtr = std::unique_ptr<corsika::geometry::Volume>;

    VolumeTreeNode(VolUPtr pVolume = nullptr)
        : fGeoVolume(std::move(pVolume)) {}

    //! convenience function equivalent to Volume::Contains
    bool Contains(corsika::geometry::Point const& p) const {
      return fGeoVolume->Contains(p);
    }

    VolumeTreeNode<IModelProperties> const* Excludes(
        corsika::geometry::Point const& p) const {
      auto exclContainsIter =
          std::find_if(fExcludedNodes.cbegin(), fExcludedNodes.cend(),
                       [&](auto const& s) { return bool(s->Contains(p)); });

      return exclContainsIter != fExcludedNodes.cend() ? *exclContainsIter : nullptr;
    }

    /** returns a pointer to the sub-VolumeTreeNode which is "responsible" for the given
     * \class Point \p p, or nullptr iff \p p is not contained in this volume.
     */
    VolumeTreeNode<IModelProperties> const* GetContainingNode(
        corsika::geometry::Point const& p) const {
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

    /**
     * Traverses the VolumeTree pre- or post-order and calls the functor  \p func for each
     * node. \p func takes a reference to VolumeTreeNode as argument. The return value \p
     * func is ignored.
     */
    template <typename TCallable, bool preorder = true>
    void walk(TCallable func) {
      if constexpr (preorder) { func(*this); }

      std::for_each(fChildNodes.begin(), fChildNodes.end(),
                    [&](auto& v) { v->walk(func); });

      if constexpr (!preorder) { func(*this); };
    }

    void AddChild(VTNUPtr pChild) {
      pChild->fParentNode = this;
      fChildNodes.push_back(std::move(pChild));
      // It is a bad idea to return an iterator to the inserted element
      // because it might get invalidated when the vector needs to grow
      // later and the caller won't notice.
    }

    void ExcludeOverlapWith(VTNUPtr const& pNode) {
      fExcludedNodes.push_back(pNode.get());
    }

    auto* GetParent() const { return fParentNode; };

    auto const& GetChildNodes() const { return fChildNodes; }

    auto const& GetExcludedNodes() const { return fExcludedNodes; }

    auto const& GetVolume() const { return *fGeoVolume; }

    auto const& GetModelProperties() const { return *fModelProperties; }

    bool HasModelProperties() const { return fModelProperties.get() != nullptr; }

    template <typename ModelProperties, typename... Args>
    auto SetModelProperties(Args&&... args) {
      static_assert(std::is_base_of_v<IModelProperties, ModelProperties>,
                    "unusable type provided");

      fModelProperties = std::make_shared<ModelProperties>(std::forward<Args>(args)...);
      return fModelProperties;
    }

    void SetModelProperties(IMPSharedPtr ptr) { fModelProperties = ptr; }

    template <class MediumType, typename... Args>
    static auto CreateMedium(Args&&... args) {
      static_assert(std::is_base_of_v<IMediumModel, MediumType>,
                    "unusable type provided, needs to be derived from \"IMediumModel\"");

      return std::make_shared<MediumType>(std::forward<Args>(args)...);
    }

  private:
    std::vector<VTNUPtr> fChildNodes;
    std::vector<VolumeTreeNode<IModelProperties> const*> fExcludedNodes;
    VolumeTreeNode<IModelProperties> const* fParentNode = nullptr;
    VolUPtr fGeoVolume;
    IMPSharedPtr fModelProperties;
  };

} // namespace corsika::environment

#endif
