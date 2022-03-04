#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/MediumProperties.hpp>
#include <corsika/media/MediumPropertyModel.hpp>

using namespace corsika;

// Example to show how to construct an environment in CORSIKA

// MediumType is constructed via "mixin inheritance"
// Here, we construct our base class with IMediumModel and IMediumPropertyModel:
// the former allow us define a density profile and nuclear composite,
// the later is used to determine energy losses parameters.
using IMediumType = IMediumPropertyModel<IMediumModel>;

using EnvType = Environment<IMediumType>;

int main() {
  // define the environment and universe
  EnvType env;
  auto *universe = env.getUniverse().get();
  auto const &rootCS = env.getCoordinateSystem();

  // create a geometry object
  Point const center{rootCS, 0_m, 0_m, 0_m};
  auto radius = 1000_m;
  auto sphere = std::make_unique<Sphere>(center, radius);

  // create node from geometry object and put it into universe
  auto node = std::make_unique<VolumeTreeNode<IMediumType>>(std::move(sphere));
  universe->addChild(std::move(node));

  // set media properties to our node, say it is water
  auto nuc_comp =
      NuclearComposition({{Code::Carbon, Code::Oxygen}, {0.11, 0.89}});
  auto density = 1_g / (1_cm * 1_cm * 1_cm);
  auto medium =
      std::make_shared<MediumPropertyModel<HomogeneousMedium<IMediumType>>>(
          Medium::WaterLiquid, density, nuc_comp);

  node->setModelProperties(medium);

  // example to explore the node
  for (LengthType h = 0_m; h < radius; h += 100_m) {
    Point const ptest{rootCS, 0_m, 0_m, h};
    auto rho = env.getUniverse()
                   ->getContainingNode(ptest)
                   ->getModelProperties()
                   .getMassDensity(ptest);
    CORSIKA_LOG_INFO("radius: {:.2f} m, density: {:.2f} g/cm^3, ", h / 1_m,
                     rho / 1_g * (1_cm * 1_cm * 1_cm));
  }

  return 0;
}