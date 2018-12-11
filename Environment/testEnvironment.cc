#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file

#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/particles/ParticleProperties.h>
#include <catch2/catch.hpp>

using namespace corsika::geometry;
using namespace corsika::environment;
using namespace corsika::units::si;

TEST_CASE("HomogeneousMedium") {
  NuclearComposition const protonComposition(
      std::vector<corsika::particles::Code>{corsika::particles::Code::Proton},
      std::vector<float>{1.f});
  HomogeneousMedium<IMediumModel> const medium(19.2_g / cube(1_cm), protonComposition);
}
