/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Volume.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <memory>
#include <vector>

namespace corsika {

  class Empty {}; //<! intended for usage as default template argument

  template <typename TModelProperties = Empty>
  class VolumeTreeNode {

  public:
    using IModelProperties = TModelProperties;
    using VTN_type = VolumeTreeNode<IModelProperties>;
    using VTNUPtr = std::unique_ptr<VolumeTreeNode<IModelProperties>>;
    using IMPSharedPtr = std::shared_ptr<IModelProperties>;
    using VolUPtr = std::unique_ptr<corsika::Volume>;

    VolumeTreeNode(VolUPtr pVolume = nullptr)
        : fGeoVolume(std::move(pVolume)) {}

    //! convenience function equivalent to Volume::Contains
    inline bool Contains(corsika::Point const& p) const;

    inline VolumeTreeNode<IModelProperties> const* Excludes(
        corsika::Point const& p) const;

    /** returns a pointer to the sub-VolumeTreeNode which is "responsible" for the given
     * \class Point \p p, or nullptr iff \p p is not contained in this volume.
     */
    inline VolumeTreeNode<IModelProperties> const* GetContainingNode(
        corsika::Point const& p) const;

    /**
     * Traverses the VolumeTree pre- or post-order and calls the functor  \p func for each
     * node. \p func takes a reference to VolumeTreeNode as argument. The return value \p
     * func is ignored.
     */
    template <typename TCallable, bool preorder = true>
    inline void walk(TCallable func);

    inline void AddChild(VTNUPtr pChild);

    inline void ExcludeOverlapWith(VTNUPtr const& pNode);

    inline auto* GetParent() const { return fParentNode; };

    inline auto const& GetChildNodes() const { return fChildNodes; }

    inline auto const& GetExcludedNodes() const { return fExcludedNodes; }

    inline auto const& GetVolume() const { return *fGeoVolume; }

    inline auto const& GetModelProperties() const { return *fModelProperties; }

    inline bool HasModelProperties() const { return fModelProperties.get() != nullptr; }

    template <typename ModelProperties, typename... Args>
    inline auto SetModelProperties(Args&&... args) {
      static_assert(std::is_base_of_v<IModelProperties, ModelProperties>,
                    "unusable model properties type provided");

      fModelProperties = std::make_shared<ModelProperties>(std::forward<Args>(args)...);
      return fModelProperties;
    }

    inline void SetModelProperties(IMPSharedPtr ptr) { fModelProperties = ptr; }

    /*
    template <class MediumType, typename... Args>
    static auto CreateMedium(Args&&... args);

  private:
    std::vector<VTNUPtr> fChildNodes;
    std::vector<VolumeTreeNode<IModelProperties> const*> fExcludedNodes;
    VolumeTreeNode<IModelProperties> const* fParentNode = nullptr;
    VolUPtr fGeoVolume;
    IMPSharedPtr fModelProperties;
  };

} // namespace corsika

#include <corsika/detail/media/VolumeTreeNode.inl>
