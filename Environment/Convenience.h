/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/SlidingPlanarExponential.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <memory>

namespace corsika::environment {

class LayeredAtmosphereBuilder {
    std::unique_ptr<NuclearComposition> nuclearComposition_;
    geometry::Point center_;
    units::si::LengthType previousRadius_{LengthType::zero()};
    static auto constexpr earthRadius_ = 6371_km;
    
public:
    LayeredAtmosphereBuilder(corsika::geometry::Point center) : center_(center) {}
    
    void setNuclearComposition(NuclearComposition composition) {
        nuclearComposition_ = std::make_unique<NuclearComposition>(composition);
    }
    
    void addLayer(units::si::GrammageType a, units::si::GrammageType b, units::si::LengthType c, units::si::LengthType thickness) {
        auto const radius = previousRadius_ + thickness;
        
        std::make_unique<BaseNodeType>(
          std::make_unique<geometry::Sphere>(center_, radius);
          
        previousRadius_ = radiusl
    }
    
    auto const assemble() {
        // get the outmost VolumeTreeNode
    }
};

}
