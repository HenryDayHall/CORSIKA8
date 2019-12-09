
#include <corsika/process/proposal/Interaction.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/units/PhysicalUnits.h>
#include <set>
#include <map>
#include <memory>
#include <limits>
#include <random>
#include <tuple>
#include <corsika/utl/COMBoost.h>

namespace corsika::process::proposal {

using namespace corsika::setup;
using namespace corsika::environment;
using namespace corsika::units::si;
using namespace corsika;

using SetupParticle = setup::Stack::StackIterator;

template <>
Interaction<SetupEnvironment>::Interaction(SetupEnvironment const& env, ParticleCut const& cut)
      : fEnvironment(env),
        pCut(cut)
{
    using namespace corsika::particles;
    using namespace units::si;

    auto& universe = *(fEnvironment.GetUniverse());

    auto const allCompositionsInUniverse = std::invoke([&]() {
      std::vector<NuclearComposition> allCompositionsInUniverse;
      auto collectCompositions = [&](auto& vtn) {
        if (vtn.HasModelProperties()) {
          auto const& comp =
              vtn.GetModelProperties().GetNuclearComposition();
          allCompositionsInUniverse.push_back(comp);
        }
      };
      universe.walk(collectCompositions);
      return allCompositionsInUniverse;
    });


    std::map<const NuclearComposition*, PROPOSAL::Medium> composition_medium_map;
    for (const NuclearComposition& ncarg : allCompositionsInUniverse) {
        std::vector<particles::Code> pcode { ncarg.GetComponents() };
        std::vector<float> fractions { ncarg.GetFractions() };
        std::vector<std::shared_ptr<PROPOSAL::Components::Component>> comp_vec;

        for(unsigned int i=0; i< pcode.size(); i++ ) {
            comp_vec.push_back(
				std::make_shared<PROPOSAL::Components::Component>(
				  PROPOSAL::Components::Component(
					GetName(pcode[i]),
					GetNucleusZ(pcode[i]),
					GetNucleusA(pcode[i]),
					fractions[i])
				)
			);
        }

        // use ionization constants from air, until CORSIKA implements them
		PROPOSAL::Air tmp_air;
        const PROPOSAL::Medium air(
            tmp_air.GetName(),
            1., // density correction set to 1 because it cancels out
            tmp_air.GetI(),
            tmp_air.GetC(),
            tmp_air.GetA(),
            tmp_air.GetM(),
            tmp_air.GetX0(),
            tmp_air.GetX1(),
            tmp_air.GetD0(),
            1.0,// massDensity set to 1, because it cancels out
           comp_vec
        );

        composition_medium_map[&ncarg] =  air;
    }

	const double e_cut = 500.;
	const double v_cut = -1.;
    PROPOSAL::EnergyCutSettings loss_cut(e_cut,v_cut); // energy_loss, relatic_energy loss (negativ means unused)

    std::string bremsstrahlung_param_str ="BremsKelnerKokoulinPetrukhin";
    std::string photonuclear_param_str = "PhotoAbramowiczLevinLevyMaor97";
    std::string epairproduction_param_str = "EpairKelnerKokoulinPetrukhin";
    std::string ionization_param_str = "IonizBetheBlochRossi"; // IonizBergerSeltzerBhabha IonizBergerSeltzerMollerg
	std::string shadowing_str = "ShadowButkevichMikhailov";

    std::string annihilation_param_str = "None"; // Heitler
    std::string compton_param_str = "None"; // KleinNishina
    std::string muPair_param_str = "None"; // KelnerKokoulinPetrukhin
	std::string photoPair_param_str = "None"; // Tsai
    std::string weak_param_str = "None"; // CooperSarkarMertsch

	PROPOSAL::Utility::Definition utility_def;
    utility_def.brems_def.lpm_effect = false;
    utility_def.epair_def.lpm_effect = false;
	utility_def.photo_def.hard_component = false;
	utility_def.photo_def.shadow = PROPOSAL::PhotonuclearFactory::Get().GetShadowEnumFromString(shadowing_str);
    utility_def.brems_def.parametrization = PROPOSAL::BremsstrahlungFactory::Get().GetEnumFromString(bremsstrahlung_param_str);
    utility_def.epair_def.parametrization = PROPOSAL::EpairProductionFactory::Get().GetEnumFromString(epairproduction_param_str);
    utility_def.ioniz_def.parametrization = PROPOSAL::IonizationFactory::Get().GetEnumFromString(ionization_param_str);
    utility_def.photo_def.parametrization = PROPOSAL::PhotonuclearFactory::Get().GetEnumFromString(photonuclear_param_str);

	utility_def.annihilation_def.parametrization = PROPOSAL::AnnihilationFactory::Get().GetEnumFromString(annihilation_param_str);
	utility_def.compton_def.parametrization = PROPOSAL::ComptonFactory::Get().GetEnumFromString(compton_param_str);
	utility_def.mupair_def.parametrization = PROPOSAL::MupairProductionFactory::Get().GetEnumFromString(muPair_param_str);
	utility_def.photopair_def.parametrization = PROPOSAL::PhotoPairFactory::Get().GetEnumFromString(photoPair_param_str);
	utility_def.weak_def.parametrization = PROPOSAL::WeakInteractionFactory::Get().GetEnumFromString(weak_param_str);

	PROPOSAL::InterpolationDef interpolation_def;
	interpolation_def.path_to_tables = "~/.local/share/PROPOSAL/tables";
	interpolation_def.path_to_tables_readonly = "~/.local/share/PROPOSAL/tables";
	interpolation_def.order_of_interpolation = 5; // order of interpolation
	interpolation_def.max_node_energy = 1e14; // upper energy bound for Interpolation (MeV)
	interpolation_def.nodes_cross_section = 100; // number of interpolation in cross section
	interpolation_def.nodes_propagate = 1000; // number of interpolation in propagate
	interpolation_def.do_binary_tables = true;
	interpolation_def.just_use_readonly_path = false;

	// std::map<particles::Code, double> corsika_particle_to_utility_map;


    std::cout << "go PROPOSAL !!!" << std::endl;
	for(auto const& medium:composition_medium_map){
		corsika_particle_to_utility_map[Code::MuMinus] = std::make_unique<PROPOSAL::UtilityInterpolantInteraction>(
			PROPOSAL::UtilityInterpolantInteraction(PROPOSAL::Utility(PROPOSAL::MuMinusDef::Get(), medium.second, loss_cut, utility_def, interpolation_def),
													interpolation_def)
		);
		corsika_particle_to_displacement_map[Code::MuMinus] = std::make_unique<PROPOSAL::UtilityInterpolantDisplacement>(
			PROPOSAL::UtilityInterpolantDisplacement(PROPOSAL::Utility(PROPOSAL::MuMinusDef::Get(), medium.second, loss_cut, utility_def, interpolation_def),
										interpolation_def)
		);
	}
    std::cout << "PROPOSAL done." << std::endl;

}

// Interaction::~Interaction {}

// template <>
// template <>
// corsika::process::EProcessReturn Interaction<SetupEnvironment>::DoInteraction(SetupParticle const& vP) {

//     // double rndi = dist(rng);
//     // double rndiMin = corsika_particle_to_utility_map[pcode]->Calculate(initial_energy, p_low, 0);

//     // if (rndd >= rnddMin || rnddMin <= 0) {
//     //     // return particle_.GetLow();
//     //     return process::EProcessReturn::eOk;
//     // } else {
//     //     return corsika_particle_to_utility_map[pcode]->GetUpperLimit(initial_energy, rndi) ;
//     // }

//     return process::EProcessReturn::eOk;
// }

template <>
template <>
corsika::units::si::GrammageType Interaction<SetupEnvironment>::GetInteractionLength(SetupParticle const& vP) {
        std::uniform_real_distribution<double> distribution(0., 1.);
        double rndi = distribution(fRNG);
        double particle_energy = vP.GetEnergy()  / 1_GeV * 1000;
        double rndiMin = corsika_particle_to_utility_map[vP.GetPID()]->Calculate(particle_energy, 0, 0);

        if (rndi >= rndiMin || rndiMin <= 0) {
            return  std::numeric_limits<double>::max() * 1_g / 1_cm / 1_cm;
        }
        double final_energy = corsika_particle_to_utility_map[vP.GetPID()]->GetUpperLimit(particle_energy, rndi) ;
        return corsika_particle_to_displacement_map[vP.GetPID()]->Calculate(
                    particle_energy,
                    final_energy,
                    std::numeric_limits<double>::max(),
                    PROPOSAL::Vector3D(),
                    PROPOSAL::Vector3D()
                ) * 1_g / 1_cm / 1_cm;
}

template <>
template <>
process::EProcessReturn Interaction<SetupEnvironment>::DoInteraction(SetupParticle& vP) {
    std::uniform_real_distribution<double> distribution(0., 1.);
    double rnd1 = distribution(fRNG);
    double rnd2 = distribution(fRNG);
    double rnd3 = distribution(fRNG);
    double particle_energy = vP.GetEnergy()  / 1_GeV * 1000;

	corsika::geometry::Point pOrig = vP.GetPosition();
	corsika::units::si::TimeType tOrig = vP.GetTime();
	corsika::stack::MomentumVector Ptot = vP.GetMomentum();
    corsika::geometry::QuantityVector q_momentum = Ptot.GetComponents();
	PROPOSAL::Vector3D initial_direction(q_momentum[0] / 1_GeV, q_momentum[1] / 1_GeV, q_momentum[2] / 1_GeV);
	initial_direction.normalise();
	initial_direction.CalculateSphericalCoordinates();
	std::pair<double, PROPOSAL::DynamicData::Type> energy_loss;
	energy_loss = corsika_particle_to_utility_map[vP.GetPID()]->GetUtility().StochasticLoss(particle_energy, rnd1, rnd2, rnd3);

	std::vector<PROPOSAL::CrossSection*> cross_sections = corsika_particle_to_utility_map[vP.GetPID()]->GetUtility().GetCrosssections();
    std::pair<std::vector<PROPOSAL::Particle*>, bool> products;
	for (unsigned int i = 0; i < cross_sections.size(); i++) {
		if (cross_sections[i]->GetTypeId() == energy_loss.second)
		{
			// calculate produced particles, get an empty list if no particles
            // are produced in interaction
			products = cross_sections[i]->CalculateProducedParticles(particle_energy, energy_loss.first, initial_direction);
            break;
		}
	}
    double particle_energy_after_loss = particle_energy - energy_loss.first;
    if (particle_energy_after_loss == 0.)
    {
        PROPOSAL::Vector3D particle_direction = initial_direction * particle_energy_after_loss;

        corsika::geometry::QuantityVector q_vec(
            particle_direction.GetX() * 1_MeV,
            particle_direction.GetY() * 1_MeV,
            particle_direction.GetZ() * 1_MeV);
        // vp.GetCoordinateSystem();
        const corsika::geometry::CoordinateSystem& rootCS = corsika::geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
        corsika::stack::MomentumVector p_particle(rootCS, q_vec);
        // std::cout << p_particle.GetTimeLikeComponent() << " and " << particle_energy_after_loss << std::endl;
        units::si::HEPEnergyType const converted_particle_energy = particle_energy * 1_GeV;
        HEPEnergyType const eCoM = particle_energy * 1_GeV ;
        //auto const Plab = boost.fromCoM(FourVector(eCoM, p_particle));
	auto pnew = vP.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
								geometry::Point, units::si::TimeType>
                                {
                                 //convert_proposal_particle_name_to_corsika_code[PROPOSAL::MuMinusDef::Get().name],
                                 vP.GetPID(),
                                 eCoM,
                                 p_particle,
                                 pOrig,
                                 tOrig
                                 }
				);
    }
	// // position and time of interaction, not used in PROPOSAL

	// // add to corsika stack
	// auto pnew = vP.AddSecondary(tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
	// 							geometry::Point, units::si::TimeType>
	// 							{
	// 							  process::sibyll::ConvertFromSibyll(psib.GetPID()),
	// 							  energy_lost,
                  				  // Plab.GetTimeLikeComponent(),
	// 							  Plab.GetSpaceLikeComponents(),
	// 							  pOrig,
	// 							  tOrig
	// 							}
	// 			);

	return process::EProcessReturn::eOk;
}

} // corsika::process::proposal
