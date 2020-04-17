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

#include <stdexcept>
#include <vector>
#include <corsika/framework/stack/Stack.hpp>

namespace corsika {



 // template <typename StackDataType, template <typename> typename ParticleInterface>
 // SecondaryView<StackDataType, ParticleInterface>:



	template <typename StackDataType, template <typename> typename ParticleInterface>
    template <typename... Args>
    auto SecondaryView<StackDataType, ParticleInterface>::AddSecondary(const Args... v) {
      StackIterator proj = GetProjectile();
      return AddSecondary(proj, v...);
    }

	template <typename StackDataType, template <typename> typename ParticleInterface>
    template <typename... Args>
    auto SecondaryView<StackDataType, ParticleInterface>::AddSecondary(StackIterator& proj, const Args... v) {
      // make space on stack
      InnerStackType::GetStackData().IncrementSize();
      // get current number of secondaries on stack
      const unsigned int idSec = GetSize();
      // determine index on (inner) stack where new particle will be located
      const unsigned int index = InnerStackType::GetStackData().GetSize() - 1;
      fIndices.push_back(index);
      // NOTE: "+1" is since "0" is special marker here for PROJECTILE, see
      // GetIndexFromIterator
      return StackIterator(*this, idSec + 1, proj, v...);
    }



	template <typename StackDataType, template <typename> typename ParticleInterface>
    void SecondaryView<StackDataType, ParticleInterface>::Delete(StackIterator p) {
      if (IsEmpty()) { /* error */
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      const int innerSize = InnerStackType::GetSize();
      const int innerIndex = GetIndexFromIterator(p.GetIndex());
      if (innerIndex < innerSize - 1)
        InnerStackType::GetStackData().Copy(innerSize - 1,
                                            GetIndexFromIterator(p.GetIndex()));
      DeleteLast();
    }

    template <typename StackDataType, template <typename> typename ParticleInterface>
    void SecondaryView<StackDataType, ParticleInterface>::Delete(ParticleInterfaceType p) { Delete(p.GetIterator()); }

   template <typename StackDataType, template <typename> typename ParticleInterface>
    void SecondaryView<StackDataType, ParticleInterface>::DeleteLast() {
      fIndices.pop_back();
      InnerStackType::GetStackData().DecrementSize();
    }

   template <typename StackDataType, template <typename> typename ParticleInterface>
   unsigned int SecondaryView<StackDataType, ParticleInterface>::GetIndexFromIterator(const unsigned int vI) const {
      if (vI == 0) return fProjectileIndex;
      return fIndices[vI - 1];
    }


} // namespace corsika

