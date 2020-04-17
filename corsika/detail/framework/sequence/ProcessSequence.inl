/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/sequence/BaseProcess.hpp>
#include <corsika/framework/sequence/BoundaryCrossingProcess.hpp>
#include <corsika/framework/sequence/DecayProcess.hpp>
#include <corsika/framework/sequence/InteractionProcess.hpp>
#include <corsika/framework/sequence/StackProcess.hpp>
#include <cmath>
#include <limits>
#include <type_traits>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/sequence/ContinuousProcess.hpp>
#include <corsika/framework/sequence/ProcessReturn.hpp>
#include <corsika/framework/sequence/SecondariesProcess.hpp>

namespace corsika {



	template <typename T1, typename T2>
	template <typename Particle, typename VTNType>
	EProcessReturn ProcessSequence<T1, T2>::DoBoundaryCrossing(Particle& p, VTNType const& from,
			VTNType const& to) {
		EProcessReturn ret = EProcessReturn::eOk;

		if constexpr (std::is_base_of<BoundaryCrossingProcess<T1type>, T1type>::value ||
				t1ProcSeq) {
			ret |= A.DoBoundaryCrossing(p, from, to);
		}

		if constexpr (std::is_base_of<BoundaryCrossingProcess<T2type>, T2type>::value ||
				t2ProcSeq) {
			ret |= B.DoBoundaryCrossing(p, from, to);
		}

		return ret;
	}

	template <typename T1, typename T2>
	template <typename TParticle, typename TTrack>
	EProcessReturn ProcessSequence<T1, T2>::DoContinuous(TParticle& vP, TTrack& vT) {
		EProcessReturn ret = EProcessReturn::eOk;
		if constexpr (std::is_base_of<ContinuousProcess<T1type>, T1type>::value || t1ProcSeq) {

			ret |= A.DoContinuous(vP, vT);
		}
		if constexpr (std::is_base_of<ContinuousProcess<T2type>, T2type>::value || t2ProcSeq) {
			ret |= B.DoContinuous(vP, vT);
		}
		return ret;
	}

	template <typename T1, typename T2>
	template <typename TSecondaries>
	EProcessReturn ProcessSequence<T1, T2>::DoSecondaries(TSecondaries& vS) {
		EProcessReturn ret = EProcessReturn::eOk;
		if constexpr (std::is_base_of<SecondariesProcess<T1type>, T1type>::value || t1ProcSeq) {
			ret |= A.DoSecondaries(vS);
		}
		if constexpr (std::is_base_of<SecondariesProcess<T2type>, T2type>::value || t2ProcSeq) {
			ret |= B.DoSecondaries(vS);
		}
		return ret;
	}

	template <typename T1, typename T2>
	bool ProcessSequence<T1, T2>::CheckStep() {
		bool ret = false;
		if constexpr (std::is_base_of<StackProcess<T1type>, T1type>::value || t1ProcSeq) {
			ret |= A.CheckStep();
		}
		if constexpr (std::is_base_of<StackProcess<T2type>, T2type>::value || t2ProcSeq) {
			ret |= B.CheckStep();
		}
		return ret;
	}

	template <typename T1, typename T2>
	template <typename TStack>
	EProcessReturn ProcessSequence<T1, T2>::DoStack(TStack& vS) {
		EProcessReturn ret = EProcessReturn::eOk;
		if constexpr (std::is_base_of<StackProcess<T1type>, T1type>::value || t1ProcSeq) {
			if (A.CheckStep()) { ret |= A.DoStack(vS); }
		}
		if constexpr (std::is_base_of<StackProcess<T2type>, T2type>::value || t2ProcSeq) {
			if (B.CheckStep()) { ret |= B.DoStack(vS); }
		}
		return ret;
	}

	template <typename T1, typename T2>
	template <typename TParticle, typename TTrack>
	corsika::units::si::LengthType ProcessSequence<T1, T2>::MaxStepLength(TParticle& vP, TTrack& vTrack) {
		corsika::units::si::LengthType
		max_length = // if no other process in the sequence implements it
		std::numeric_limits<double>::infinity() * corsika::units::si::meter;

		if constexpr (std::is_base_of<ContinuousProcess<T1type>, T1type>::value || t1ProcSeq) {
			corsika::units::si::LengthType const len = A.MaxStepLength(vP, vTrack);
			max_length = std::min(max_length, len);
		}
		if constexpr (std::is_base_of<ContinuousProcess<T2type>, T2type>::value || t2ProcSeq) {
			corsika::units::si::LengthType const len = B.MaxStepLength(vP, vTrack);
			max_length = std::min(max_length, len);
		}
		return max_length;
	}

	template <typename T1, typename T2>
	template <typename TParticle>
	corsika::units::si::GrammageType ProcessSequence<T1, T2>::GetTotalInteractionLength(TParticle& vP) {
		return 1. / GetInverseInteractionLength(vP);
	}

	template <typename T1, typename T2>
	template <typename TParticle>
	corsika::units::si::InverseGrammageType ProcessSequence<T1, T2>::GetTotalInverseInteractionLength(
			TParticle& vP) {
		return GetInverseInteractionLength(vP);
	}

	template <typename T1, typename T2>
	template <typename TParticle>
	corsika::units::si::InverseGrammageType ProcessSequence<T1, T2>::GetInverseInteractionLength(TParticle& vP) {
		corsika::units::si::InverseGrammageType tot = 0 * corsika::units::si::meter * corsika::units::si::meter / corsika::units::si::gram;

		if constexpr (std::is_base_of<InteractionProcess<T1type>, T1type>::value || t1ProcSeq ||
				t1SwitchProc) {
			tot += A.GetInverseInteractionLength(vP);
		}
		if constexpr (std::is_base_of<InteractionProcess<T2type>, T2type>::value || t2ProcSeq ||
				t2SwitchProc) {
			tot += B.GetInverseInteractionLength(vP);
		}
		return tot;
	}

	template <typename T1, typename T2>
	template <typename TParticle, typename TSecondaries>
	EProcessReturn ProcessSequence<T1, T2>::SelectInteraction(
			TParticle& vP, TSecondaries& vS,
			[[maybe_unused]] corsika::units::si::InverseGrammageType lambda_select,
			corsika::units::si::InverseGrammageType& lambda_inv_count)
	{

		if constexpr (t1ProcSeq || t1SwitchProc) {
			// if A is a process sequence --> check inside
			const EProcessReturn ret =
					A.SelectInteraction(vP, vS, lambda_select, lambda_inv_count);
			// if A did succeed, stop routine
			if (ret != EProcessReturn::eOk) { return ret; }
		} else if constexpr (std::is_base_of<InteractionProcess<T1type>, T1type>::value ) {
			// if this is not a ContinuousProcess --> evaluate probability
			lambda_inv_count += A.GetInverseInteractionLength(vP);
			// check if we should execute THIS process and then EXIT
			if (lambda_select < lambda_inv_count) {
				A.DoInteraction(vS);
				return EProcessReturn::eInteracted;
			}
		} // end branch A

		if constexpr (t2ProcSeq || t2SwitchProc) {
			// if A is a process sequence --> check inside
			const EProcessReturn ret =
					B.SelectInteraction(vP, vS, lambda_select, lambda_inv_count);
			// if A did succeed, stop routine
			if (ret != EProcessReturn::eOk) { return ret; }
		} else if constexpr (std::is_base_of<InteractionProcess<T2type>, T2type>::value) {
			// if this is not a ContinuousProcess --> evaluate probability
			lambda_inv_count += B.GetInverseInteractionLength(vP);
			// check if we should execute THIS process and then EXIT
			if (lambda_select < lambda_inv_count) {
				B.DoInteraction(vS);
				return EProcessReturn::eInteracted;
			}
		} // end branch A
		return EProcessReturn::eOk;
	}

	template <typename T1, typename T2>
	template <typename TParticle>
	corsika::units::si::TimeType ProcessSequence<T1, T2>::GetTotalLifetime(TParticle& p) {
		return 1. / GetInverseLifetime(p);
	}

	template <typename T1, typename T2>
	template <typename TParticle>
	corsika::units::si::InverseTimeType ProcessSequence<T1, T2>::GetTotalInverseLifetime(TParticle& p) {
		return GetInverseLifetime(p);
	}

	template <typename T1, typename T2>
	template <typename TParticle>
	corsika::units::si::InverseTimeType ProcessSequence<T1, T2>::GetInverseLifetime(TParticle& p) {
		corsika::units::si::InverseTimeType tot = 0 / corsika::units::si::second;

		if constexpr (std::is_base_of<DecayProcess<T1type>, T1type>::value || t1ProcSeq) {
			tot += A.GetInverseLifetime(p);
		}
		if constexpr (std::is_base_of<DecayProcess<T2type>, T2type>::value || t2ProcSeq) {
			tot += B.GetInverseLifetime(p);
		}
		return tot;
	}


	template <typename T1, typename T2>
	template <typename TParticle, typename TSecondaries>
	EProcessReturn ProcessSequence<T1, T2>::SelectDecay(
			TParticle& vP, TSecondaries& vS,
			[[maybe_unused]] corsika::units::si::InverseTimeType decay_select,
			corsika::units::si::InverseTimeType& decay_inv_count) {
		if constexpr (t1ProcSeq) {
			// if A is a process sequence --> check inside
			const EProcessReturn ret = A.SelectDecay(vP, vS, decay_select, decay_inv_count);
			// if A did succeed, stop routine
			if (ret != EProcessReturn::eOk) { return ret; }
		} else if constexpr (std::is_base_of<DecayProcess<T1type>, T1type>::value) {
			// if this is not a ContinuousProcess --> evaluate probability
			decay_inv_count += A.GetInverseLifetime(vP);
			// check if we should execute THIS process and then EXIT
			if (decay_select < decay_inv_count) { // more pedagogical: rndm_select <
				// decay_inv_count / decay_inv_tot
				A.DoDecay(vS);
				return EProcessReturn::eDecayed;
			}
		} // end branch A

		if constexpr (t2ProcSeq) {
			// if A is a process sequence --> check inside
			const EProcessReturn ret = B.SelectDecay(vP, vS, decay_select, decay_inv_count);
			// if A did succeed, stop routine
			if (ret != EProcessReturn::eOk) { return ret; }
		} else if constexpr (std::is_base_of<DecayProcess<T2type>, T2type>::value) {
			// if this is not a ContinuousProcess --> evaluate probability
			decay_inv_count += B.GetInverseLifetime(vP);
			// check if we should execute THIS process and then EXIT
			if (decay_select < decay_inv_count) {
				B.DoDecay(vS);
				return EProcessReturn::eDecayed;
			}
		} // end branch B
		return EProcessReturn::eOk;
	}


} // namespace corsika

