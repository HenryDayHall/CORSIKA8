/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/logging/Logging.h>
#include <corsika/stack/Stack.h>

#include <tuple>
#include <utility>
#include <vector>

namespace corsika::stack::node {

  /**
   * @class GeometryDataInterface
   *
   * corresponding defintion of a stack-readout object, the iteractor
   * dereference operator will deliver access to these function
  // defintion of a stack-readout object, the iteractor dereference
  // operator will deliver access to these function
   */
  template <typename T, typename TEnvType>
  class GeometryDataInterface : public T {

  protected:
    using T::GetStack;
    using T::GetStackData;

  public:
    using T::GetIndex;
    using BaseNodeType = typename TEnvType::BaseNodeType;

  public:
    // default version for particle-creation from input data
    void SetParticleData(const std::tuple<BaseNodeType const*> v) {
      SetNode(std::get<0>(v));
    }
    void SetParticleData(GeometryDataInterface& parent,
                         const std::tuple<BaseNodeType const*>) {
      SetNode(parent.GetNode()); // copy Node from parent particle!
    }
    void SetParticleData() { SetNode(nullptr); }
    void SetParticleData(GeometryDataInterface& parent) {
      SetNode(parent.GetNode()); // copy Node from parent particle!
    }

    std::string as_string() const { return fmt::format("node={}", fmt::ptr(GetNode())); }

    void SetNode(BaseNodeType const* v) { GetStackData().SetNode(GetIndex(), v); }
    BaseNodeType const* GetNode() const { return GetStackData().GetNode(GetIndex()); }
  };

  // definition of stack-data object to store geometry information
  template <typename TEnvType>

  /**
   * @class GeometryData
   *
   * definition of stack-data object to store geometry information
   */
  class GeometryData {

  public:
    using BaseNodeType = typename TEnvType::BaseNodeType;

    // these functions are needed for the Stack interface
    void Clear() { fNode.clear(); }
    unsigned int GetSize() const { return fNode.size(); }
    unsigned int GetCapacity() const { return fNode.size(); }
    void Copy(const int i1, const int i2) { fNode[i2] = fNode[i1]; }
    void Swap(const int i1, const int i2) { std::swap(fNode[i1], fNode[i2]); }

    // custom data access function
    void SetNode(const int i, BaseNodeType const* v) { fNode[i] = v; }
    BaseNodeType const* GetNode(const int i) const { return fNode[i]; }

    // these functions are also needed by the Stack interface
    void IncrementSize() { fNode.push_back(nullptr); }
    void DecrementSize() {
      if (fNode.size() > 0) { fNode.pop_back(); }
    }

    // custom private data section
  private:
    std::vector<const BaseNodeType*> fNode;
  };

  template <typename T, typename TEnv>
  struct MakeGeometryDataInterface {
    typedef GeometryDataInterface<T, TEnv> type;
  };

} // namespace corsika::stack::node
