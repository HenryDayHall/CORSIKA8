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
    using VolUPtr = std::unique_ptr<IVolume>;

    VolumeTreeNode(VolUPtr pVolume = nullptr)
        : geoVolume_(std::move(pVolume)) {}

    //! convenience function equivalent to Volume::isInside
    inline bool isInside(Point const& p) const;

    inline VolumeTreeNode<IModelProperties> const* isExcluded(Point const& p) const;

    /** returns a pointer to the sub-VolumeTreeNode which is "responsible" for the given
     * \class Point \p p, or nullptr iff \p p is not contained in this volume.
     */
    inline VolumeTreeNode<IModelProperties> const* getContainingNode(
        Point const& p) const;

    /**
     * Traverses the VolumeTree pre- or post-order and calls the functor  \p func for each
     * node. \p func takes a reference to VolumeTreeNode as argument. The return value \p
     * func is ignored.
     */
    template <typename TCallable, bool preorder = true>
    inline void walk(TCallable func);

    inline void addChild(VTNUPtr pChild);

    inline void excludeOverlapWith(VTNUPtr const& pNode);

    inline auto* getParent() const { return parentNode_; };

    inline auto const& getChildNodes() const { return childNodes_; }

    inline auto const& getExcludedNodes() const { return excludedNodes_; }

    inline auto const& getVolume() const { return *geoVolume_; }

    inline auto const& getModelProperties() const { return *modelProperties_; }

    inline bool hasModelProperties() const { return modelProperties_.get() != nullptr; }

    template <typename ModelProperties, typename... Args>
    inline auto setModelProperties(Args&&... args) {
      static_assert(std::is_base_of_v<IModelProperties, ModelProperties>,
                    "unusable model properties type provided");

      modelProperties_ = std::make_shared<ModelProperties>(std::forward<Args>(args)...);
      return modelProperties_;
    }

    inline void setModelProperties(IMPSharedPtr ptr) { modelProperties_ = ptr; }

    /*
    template <class MediumType, typename... Args>
    static auto createMedium(Args&&... args);

  private:
    std::vector<VTNUPtr> childNodes_;
    std::vector<VolumeTreeNode<IModelProperties> const*> excludedNodes_;
    VolumeTreeNode<IModelProperties> const* parentNode_ = nullptr;
    VolUPtr geoVolume_;
    IMPSharedPtr modelProperties_;
  };

} // namespace corsika

#include <corsika/detail/media/VolumeTreeNode.inl>
