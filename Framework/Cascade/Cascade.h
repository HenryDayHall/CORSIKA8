
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */


#ifndef _include_corsika_cascade_Cascade_h_
#define _include_corsika_cascade_Cascade_h_

#include <corsika/environment/Environment.h>
#include <corsika/process/ProcessReturn.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

#include <cmath>
#include <iostream>

namespace corsika::cascade {

  template <typename Tracking, typename ProcessList, typename Stack>
  class Cascade {
    using Particle = typename Stack::ParticleType;

    Cascade() = delete;

  public:
    Cascade(corsika::environment::Environment const& env, Tracking& tr, ProcessList& pl,
            Stack& stack)
        : fEnvironment(env)
        , fTracking(tr)
        , fProcessSequence(pl)
        , fStack(stack) {}

    void Init() {
      fTracking.Init();
      fProcessSequence.Init();
      fStack.Init();
    }

    void Run() {
      while (!fStack.IsEmpty()) {
        while (!fStack.IsEmpty()) {
          auto pNext = fStack.GetNextParticle();
          Step(pNext);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations();
      }
    }

  private:
    void Step(Particle& particle) {
      using namespace corsika::units::si;

      // determine geometric tracking
      corsika::setup::Trajectory step = fTracking.GetTrack(particle);

      // determine combined total interaction length (inverse)
      InverseGrammageType const total_inv_lambda =
          fProcessSequence.GetTotalInverseInteractionLength(particle, step);

      // sample random exponential step length in grammage
      std::exponential_distribution expDist(total_inv_lambda * (1_g / (1_m * 1_m)));
      GrammageType const next_interact = (1_g / (1_m * 1_m)) * expDist(fRNG);

      std::cout << "total_inv_lambda=" << total_inv_lambda
                << ", next_interact=" << next_interact << std::endl;

      // convert next_step from grammage to length
      auto const* currentNode =
          fEnvironment.GetUniverse()->GetContainingNode(particle.GetPosition());

      if (currentNode == &*fEnvironment.GetUniverse()) {
        throw std::runtime_error("particle entered void universe");
      }

      LengthType const distance_interact =
          currentNode->GetModelProperties().ArclengthFromGrammage(step, next_interact);

      // determine the maximum geometric step length
      LengthType const distance_max = fProcessSequence.MaxStepLength(particle, step);
      std::cout << "distance_max=" << distance_max << std::endl;

      // determine combined total inverse decay time
      InverseTimeType const total_inv_lifetime =
          fProcessSequence.GetTotalInverseLifetime(particle);

      // sample random exponential decay time
      std::exponential_distribution expDistDecay(total_inv_lifetime * 1_s);
      TimeType const next_decay = 1_s * expDistDecay(fRNG);
      std::cout << "total_inv_lifetime=" << total_inv_lifetime
                << ", next_decay=" << next_decay << std::endl;

      // convert next_decay from time to length [m]
      LengthType const distance_decay = next_decay * particle.GetMomentum().norm() /
                                        particle.GetEnergy() *
                                        corsika::units::constants::c;

      // take minimum of geometry, interaction, decay for next step
      auto const min_distance =
          std::min({distance_interact, distance_decay, distance_max});

      std::cout << " move particle by : " << min_distance << std::endl;

      // here the particle is actually moved along the trajectory to new position:
      // std::visit(corsika::setup::ParticleUpdate<Particle>{particle}, step);
      particle.SetPosition(step.PositionFromArclength(min_distance));
      // .... also update time, momentum, direction, ...

      step.LimitEndTo(min_distance);

      // apply all continuous processes on particle + track
      corsika::process::EProcessReturn status =
          fProcessSequence.DoContinuous(particle, step, fStack);

      if (status == corsika::process::EProcessReturn::eParticleAbsorbed) {
        std::cout << "Cascade: delete absorbed particle " << particle.GetPID() << " "
                  << particle.GetEnergy() / 1_GeV << "GeV" << std::endl;
        particle.Delete();
        return;
      }

      std::cout << "sth. happening before geometric limit ? "
                << ((min_distance < distance_max) ? "yes" : "no") << std::endl;

      if (min_distance < distance_max) { // interaction to happen within geometric limit
        // check whether decay or interaction limits this step

        if (min_distance == distance_interact) {
          std::cout << "collide" << std::endl;

          InverseGrammageType const actual_inv_length =
              fProcessSequence.GetTotalInverseInteractionLength(particle, step);

          corsika::random::UniformRealDistribution<InverseGrammageType> uniDist(
              actual_inv_length);
          const auto sample_process = uniDist(fRNG);
          InverseGrammageType inv_lambda_count = 0. * meter * meter / gram;
          fProcessSequence.SelectInteraction(particle, fStack, sample_process,
                                             inv_lambda_count);
        } else {
          std::cout << "decay" << std::endl;
          InverseTimeType const actual_decay_time =
              fProcessSequence.GetTotalInverseLifetime(particle);

          corsika::random::UniformRealDistribution<InverseTimeType> uniDist(
              actual_decay_time);
          const auto sample_process = uniDist(fRNG);
          InverseTimeType inv_decay_count = 0 / second;
          fProcessSequence.SelectDecay(particle, fStack, sample_process, inv_decay_count);
        }
      }
    }

  private:
    corsika::environment::Environment const& fEnvironment;
    Tracking& fTracking;
    ProcessList& fProcessSequence;
    Stack& fStack;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");
  };

} // namespace corsika::cascade

#endif
