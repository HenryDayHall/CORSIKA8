n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <cfenv>

/*
 * Same declaration of function as provided in GLIBC
 * Repetition allowed in the case where cfenv defines the functions already, no clash.
 */
extern "C" {

int feenableexcept(int excepts);
int fedisableexcept(int excepts);
}

#ifdef CORSIKA_HAS_FEENABLEEXCEPT
    // Nothing to do, OS privides the functions
#elif
    #ifdef CORSIKA_OS_MAC
        #include <corsika/detail/framework/utility/CorsikaFenvOSX.inl>
    #elif
        #include <corsika/detail/framework/utility/CorsikaFenvFallback.inl>
    #endif
#endif