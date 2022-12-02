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

EMThinning::EMThinning(HEPEnergyType threshold, double maxWeight) : threshold_{threshold}, maxWeight_{maxWeight} {}

template <typename TStackView>
    void EMThinning::doSecondaries(TStackView& view) {
    if (view.size() != 2) return;
    auto particle1 = view.begin();
    auto particle2 = std::next(particle1);

    if (.getCode())
	corsika::HEPEnergyType const E0 = 
    
}

}
