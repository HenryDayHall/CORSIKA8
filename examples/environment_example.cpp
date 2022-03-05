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
  VolumeTreeNode<IMediumType>* universe = env.getUniverse().get();

  // create a geometry object: a sphere with radius of 1000m
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
  Point const center{rootCS, 0_m, 0_m, 0_m};
  LengthType radius = 1000_m;
  auto sphere = std::make_unique<Sphere>(center, radius);

  // create node from geometry object
  auto node = std::make_unique<VolumeTreeNode<IMediumType>>(std::move(sphere));

  // set medium properties to our node, say it is water
  auto nuc_comp = NuclearComposition({{Code::Hydrogen, Code::Oxygen}, {0.11, 0.89}});
  MassDensityType density = 1_g / (1_cm * 1_cm * 1_cm);
  auto medium = std::make_shared<MediumPropertyModel<HomogeneousMedium<IMediumType>>>(
      Medium::WaterLiquid, density, nuc_comp);
  node->setModelProperties(medium);

  // put our node into universe
  // node: this has to be down after setting node model properties, since
  // std::move will make our previous defined node, which is a unique pointer
  // un-referencable in the context
  universe->addChild(std::move(node));

  // example to explore the media properties of the node
  for (LengthType h = 0_m; h < radius; h += 100_m) {
    Point const ptest{rootCS, 0_m, 0_m, h};
    IMediumType const& media_prop_ =
        env.getUniverse()->getContainingNode(ptest)->getModelProperties();
    MassDensityType rho_ = media_prop_.getMassDensity(ptest);
    NuclearComposition nuc_comp_ = media_prop_.getNuclearComposition();
    std::vector<Code> nucs_ = nuc_comp_.getComponents();
    std::vector<double> fracs_ = nuc_comp_.getFractions();
    CORSIKA_LOG_INFO(
        "radius: {:.2f} m, density: {:.2f} g/cm^3, nucs: ({:d}, {:d}), fracs: ({:.2f}, "
        "{:.2f})",
        h / 1_m, rho_ / 1_g * (1_cm * 1_cm * 1_cm), get_PDG(nucs_[0]), get_PDG(nucs_[1]),
        fracs_[0], fracs_[1]);
  }

  return 0;
}