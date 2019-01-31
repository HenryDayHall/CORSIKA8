
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

#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>
#include <corsika/stack/super_stupid/SuperStupidStack.h>

// this is an auxiliary help typedef, which I don't know how to put
// into NuclearStackExtension.h where it belongs...
template<typename StackIter> using ExtendedParticleInterfaceType =
  corsika::stack::nuclear_extension::NuclearParticleInterface<corsika::stack::super_stupid::SuperStupidStack::PIType, StackIter>;

namespace corsika::setup {

  using Stack = corsika::stack::nuclear_extension::NuclearStackExtension<corsika::stack::super_stupid::SuperStupidStack, ExtendedParticleInterfaceType>;

  // typedef corsika::stack::super_stupid::SuperStupidStack Stack;
}

#endif
