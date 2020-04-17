/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

// the basic particle data stack:
#include <corsika/stack/SuperStupidStack.h>

// extension with nuclear data for Code::Nucleus
#include <corsika/stack/NuclearStackExtension.h>

// extension with geometry information for tracking
#include <corsika/media/Environment.hpp>
#include <corsika/framework/stack/CombinedStack.hpp>

#include <tuple>
#include <utility>
#include <vector>
#include <corsika/setup/SetupEnvironment.hpp>

  namespace detail {

/**
 * @class GeometryData
 *
 * definition of stack-data object to store geometry information
 */
class GeometryData {

public:
  using BaseNodeType = typename TEnvType::BaseNodeType;

  // these functions are needed for the Stack interface
  void Init() {}
  void Clear() { fNode.clear(); }
  unsigned int GetSize() const { return fNode.size(); }
  unsigned int GetCapacity() const { return fNode.size(); }
  void Copy(const int i1, const int i2) { fNode[i2] = fNode[i1]; }
  void Swap(const int i1, const int i2) { std::swap(fNode[i1], fNode[i2]); }

  // custom data access function
  void SetNode(const int i, BaseNodeType const* v) { fNode[i] = v; }
  auto const* GetNode(const int i) const { return fNode[i]; }

  // these functions are also needed by the Stack interface
  void IncrementSize() { fNode.push_back(nullptr); }
  void DecrementSize() {
    if (fNode.size() > 0) { fNode.pop_back(); }
  }

  // custom private data section
private:
  std::vector<const BaseNodeType*> fNode;
};

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

public:
  using T::GetIndex;
  using T::GetStackData;
  using T::SetParticleData;
  using BaseNodeType = typename TEnvType::BaseNodeType;

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
  void SetNode(BaseNodeType const* v) { GetStackData().SetNode(GetIndex(), v); }
  auto const* GetNode() const { return GetStackData().GetNode(GetIndex()); }
};

namespace corsika::setup {

    // the GeometryNode stack needs to know the type of geometry-nodes from the
    // environment:
    template <typename TStackIter>
    using SetupGeometryDataInterface = typename stack::node::MakeGeometryDataInterface<
        TStackIter, corsika::setup::Environment>::type;

    //
    // this is an auxiliary help typedef, which I don't know how to put
    // into NuclearStackExtension.h where it belongs...
    template <typename StackIter>
    using ExtendedParticleInterfaceType =
        corsika::nuclear_extension::NuclearParticleInterface<
            corsika::super_stupid::SuperStupidStack::PIType, StackIter>;
    //

    // the particle data stack with extra nuclear information:
    using ParticleDataStack = corsika::nuclear_extension::NuclearStackExtension<
        corsika::super_stupid::SuperStupidStack, ExtendedParticleInterfaceType>;

    // ------------------------------------------
    // Add [optional] history data to stack, too:

    // combine particle data stack with geometry information for tracking
    template <typename StackIter>
    using StackWithGeometryInterface =
        corsika::CombinedParticleInterface<ParticleDataStack::PIType,
                                                  SetupGeometryDataInterface, StackIter>;

    using StackWithGeometry =
        corsika::CombinedStack<typename ParticleDataStack::StackImpl,
                                      GeometryData<setup::SetupEnvironment>,
                                      StackWithGeometryInterface>;

  } // namespace detail

  // ---------------------------------------
  // this is the FINAL stack we use in C8:

#ifdef WITH_HISTORY

  /*
   * the version with history
   */
  using Stack = detail::StackWithHistory;
  template <typename T1, template <typename> typename M2>
  using StackViewProducer = corsika::history::HistorySecondaryProducer<T1, M2>;

  namespace detail {
    /*
      See Issue 161

      unfortunately clang does not support this in the same way (yet) as
      gcc, so we have to distinguish here. If clang cataches up, we
      could remove the clang branch here and also in
      corsika::Cascade. The gcc code is much more generic and
      universal. If we could do the gcc version, we won't had to define
      StackView globally, we could do it with MakeView whereever it is
      actually needed. Keep an eye on this!
    */
#if defined(__clang__)
    using TheStackView = corsika::stack::SecondaryView<
        typename corsika::setup::Stack::StackImpl,
        // CHECK with CLANG: corsika::setup::Stack::MPIType>;
        corsika::setup::detail::StackWithHistoryInterface, StackViewProducer>;
#elif defined(__GNUC__) || defined(__GNUG__)
    using TheStackView =
        corsika::stack::MakeView<corsika::setup::Stack, StackViewProducer>::type;
#endif
  } // namespace detail

#else // WITH_HISTORY

  /*
   * the version without history
   */
  using Stack = detail::StackWithGeometry;
  template <typename T1, template <typename> typename M2>
  using StackViewProducer = corsika::stack::DefaultSecondaryProducer<T1, M2>;

  namespace detail {
    /*
      See Issue 161

      unfortunately clang does not support this in the same way (yet) as
      gcc, so we have to distinguish here. If clang cataches up, we
      could remove the clang branch here and also in
      corsika::Cascade. The gcc code is much more generic and
      universal. If we could do the gcc version, we won't had to define
      StackView globally, we could do it with MakeView whereever it is
      actually needed. Keep an eye on this!
    */
#if defined(__clang__)
  using StackView =
      corsika::SecondaryView<typename corsika::Stack::StackImpl,
                                    corsika::detail::StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
  using StackView = corsika::MakeView<corsika::setup::Stack>::type;
#endif
  } // namespace detail

#endif

  // ---------------------------------------
  // this is the FINAL stackitertor (particle type) we use in C8:

  using StackView = detail::TheStackView;

} // namespace corsika

