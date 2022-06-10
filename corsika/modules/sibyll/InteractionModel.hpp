#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/modules/sibyll/HadronInteractionModel.hpp>
#include <corsika/modules/sibyll/NuclearInteractionModel.hpp>

namespace corsika::sibyll {
  template <typename TEnvironment>
  class InteractionModel {
  public:
    using nuclear_model_type =
        NuclearInteractionModel<TEnvironment, HadronInteractionModel>;
    InteractionModel(TEnvironment const&);

    CrossSectionType getCrossSection(Code, Code, FourMomentum const&,
                                     FourMomentum const&) const;

    template <typename TSecondaries>
    void doInteraction(TSecondaries&, Code, Code, FourMomentum const&,
                       FourMomentum const&);

    HadronInteractionModel& getHadronInteractionModel();
    nuclear_model_type& getNuclearInteractionModel();

  private:
    HadronInteractionModel hadronSibyll_;
    nuclear_model_type nuclearSibyll_;
  };
} // namespace corsika::sibyll

#include <corsika/detail/modules/sibyll/InteractionModel.inl>
