/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>

namespace corsika {

//! This process implements thinning for EM splitting processes (1 -> 2)
 
class EMThinning : public SecondariesProcess<EMThinning> {
public:
    


    /**
     * Apply thinning to secondaries.
     *
     * @tparam TStackView
     */
    template <typename TStackView>
    void doSecondaries(TStackView&);

};
}

#include <corsika/detail/process/thinning/EMThinning.inl>
