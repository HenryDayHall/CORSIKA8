#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/modules/sibyll/HadronInteractionModel.hpp>
#include <corsika/modules/sibyll/NuclearInteractionModel.hpp>

namespace corsika::sibyll {
  template <typename TEnvironment>
  InteractionModel<TEnvironment>::InteractionModel(TEnvironment const& environment)
      : hadronSibyll_{}
      , nuclearSibyll_{hadronSibyll_, environment} {}

  template <typename TEnvironment>
  HadronInteractionModel& InteractionModel<TEnvironment>::getHadronInteractionModel() {
    return hadronSibyll_;
  }

  template <typename TEnvironment>
  typename InteractionModel<TEnvironment>::nuclear_model_type&
  InteractionModel<TEnvironment>::getNuclearInteractionModel() {
    return nuclearSibyll_;
  }

  template <typename TEnvironment>
  CrossSectionType InteractionModel<TEnvironment>::getCrossSection(
      Code projCode, Code targetCode, FourMomentum const& proj4mom,
      FourMomentum const& target4mom) const {
    if (is_nucleus(projCode))
      return getNuclearInteractionModel().getCrossSection(projCode, targetCode, proj4mom,
                                                          target4mom);
    else
      return getHadronInteractionModel().getCrossSection(projCode, targetCode, proj4mom,
                                                         target4mom);
  }

  template <typename TEnvironment>
  template <typename TSecondaries>
  void InteractionModel<TEnvironment>::doInteraction(TSecondaries& view, Code projCode,
                                                     Code targetCode,
                                                     FourMomentum const& proj4mom,
                                                     FourMomentum const& target4mom) {
    if (is_nucleus(projCode))
      return getNuclearInteractionModel().doInteraction(view, projCode, targetCode,
                                                        proj4mom, target4mom);
    else
      return getHadronInteractionModel().doInteraction(view, projCode, targetCode,
                                                       proj4mom, target4mom);
  }
} // namespace corsika::sibyll
