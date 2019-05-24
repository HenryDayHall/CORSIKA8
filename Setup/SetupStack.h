/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_setup_setupstack_h_
#define _corsika_setup_setupstack_h_

// the basic particle data stack:
#include <corsika/stack/super_stupid/SuperStupidStack.h>

// extension with nuclear data for Code::Nucleus
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>

// extension with geometry information for tracking
#include <corsika/environment/Environment.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/stack/CombinedStack.h>

#include <tuple>
#include <utility>
#include <vector>

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

  namespace detail {

    //
    // this is an auxiliary help typedef, which I don't know how to put
    // into NuclearStackExtension.h where it belongs...
    template <typename StackIter>
    using ExtendedParticleInterfaceType =
        corsika::stack::nuclear_extension::NuclearParticleInterface<
            corsika::stack::super_stupid::SuperStupidStack::PIType, StackIter>;
    //

    // the particle data stack with extra nuclear information:
    using ParticleDataStack = corsika::stack::nuclear_extension::NuclearStackExtension<
        corsika::stack::super_stupid::SuperStupidStack, ExtendedParticleInterfaceType>;

    template <typename T>
    using SetupGeometryDataInterface = GeometryDataInterface<T, setup::SetupEnvironment>;

    // combine particle data stack with geometry information for tracking
    template <typename StackIter>
    using StackWithGeometryInterface =
        corsika::stack::CombinedParticleInterface<ParticleDataStack::PIType,
                                                  SetupGeometryDataInterface, StackIter>;

    using StackWithGeometry =
        corsika::stack::CombinedStack<typename ParticleDataStack::StackImpl,
                                      GeometryData<setup::SetupEnvironment>,
                                      StackWithGeometryInterface>;

  } // namespace detail

  // this is the REAL stack we use:
  using Stack = detail::StackWithGeometry;

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
      corsika::stack::SecondaryView<typename corsika::setup::Stack::StackImpl,
                                    corsika::setup::detail::StackWithGeometryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
  using StackView = corsika::stack::MakeView<corsika::setup::Stack>::type;
#endif

} // namespace corsika::setup

#endif
