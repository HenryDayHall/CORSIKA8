
/**
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

#include <corsika/process/ProcessReturn.h>
#include <corsika/random/RNGManager.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

#include <type_traits>

using namespace corsika;
using namespace corsika::units::si;

namespace corsika::cascade {

  template <typename Tracking, typename ProcessList, typename Stack>
  class Cascade {

    typedef typename Stack::ParticleType Particle;

    Cascade() = delete;

  public:
    Cascade(Tracking& tr, ProcessList& pl, Stack& stack)
        : fTracking(tr)
        , fProcesseList(pl)
        , fStack(stack) {}

    void Init() {
      fTracking.Init();
      fProcesseList.Init();
      fStack.Init();
    }

    void Run() {
      while (!fStack.IsEmpty()) {
        while (!fStack.IsEmpty()) {
          Particle& pNext = *fStack.GetNextParticle();
          Step(pNext);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations();
      }
    }

    void Step(Particle& particle) {

      // get access to random number generator
      static corsika::random::RNG& rmng =
          corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");

      // determine geometric tracking
      corsika::setup::Trajectory step = fTracking.GetTrack(particle);

      // determine combined total interaction length (inverse)
      const double total_inv_lambda =
          fProcesseList.GetTotalInverseInteractionLength(particle, step);

      // sample random exponential step length
      const double sample_step = rmng() / (double)rmng.max();
      const double next_interact = -log(sample_step) / total_inv_lambda;
      std::cout << "total_inv_lambda=" << total_inv_lambda
                << ", next_interact=" << next_interact << std::endl;

      // determine the maximum geometric step length
      const double distance_max = fProcesseList.MaxStepLength(particle, step);
      std::cout << "distance_max=" << distance_max << std::endl;

      // determine combined total inverse decay time
      const double total_inv_lifetime = fProcesseList.GetTotalInverseLifetime(particle);

      // sample random exponential decay time
      const double sample_decay = rmng() / (double)rmng.max();
      const double next_decay = -log(sample_decay) / total_inv_lifetime;
      std::cout << "total_inv_lifetime=" << total_inv_lifetime
                << ", next_decay=" << next_decay << std::endl;

      // convert next_step from grammage to length [m]
      // Environment::GetDistance(step, next_step);
      const double distance_interact = next_interact;
      // ....

      // convert next_decay from time to length [m]
      // Environment::GetDistance(step, next_decay);
      const double distance_decay = next_decay;
      // ....

      // take minimum of geometry, interaction, decay for next step
      const double distance_decay_interact = std::min(next_decay, next_interact);
      const double distance_next = std::min(distance_decay_interact, distance_max);

      /// here the particle is actually moved along the trajectory to new position:
      // std::visit(corsika::setup::ParticleUpdate<Particle>{particle}, step);
      particle.SetPosition(step.GetPosition(1));
      // .... also update time, momentum, direction, ...

      // apply all continuous processes on particle + track
      corsika::process::EProcessReturn status =
          fProcesseList.DoContinuous(particle, step, fStack);

      if (status == corsika::process::EProcessReturn::eParticleAbsorbed) {
        // fStack.Delete(particle); // TODO: check if this is really needed
      } else {

        std::cout << "select process " << (distance_decay_interact < distance_max)
                  << std::endl;
        // check if geometry limits step, then quit this step
        if (distance_decay_interact < distance_max) {
          // check weather decay or interaction limits this step
          if (distance_decay > distance_interact) {
            std::cout << "collide" << std::endl;
            const double actual_inv_length =
                fProcesseList.GetTotalInverseInteractionLength(particle, step);
            const double sample_process = rmng() / (double)rmng.max();
            double inv_lambda_count = 0;
            fProcesseList.SelectInteraction(particle, fStack, actual_inv_length,
                                            sample_process, inv_lambda_count);
          } else {
            std::cout << "decay" << std::endl;
            const double actual_decay_time =
                fProcesseList.GetTotalInverseLifetime(particle);
            const double sample_process = rmng() / (double)rmng.max();
            double inv_decay_count = 0;
            fProcesseList.SelectDecay(particle, fStack, actual_decay_time, sample_process,
                                      inv_decay_count);
          }
        }
      }
    }

  private:
    Tracking& fTracking;
    ProcessList& fProcesseList;
    Stack& fStack;
  };

} // namespace corsika::cascade

#endif
