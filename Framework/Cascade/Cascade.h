
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
#include <corsika/units/PhysicalUnits.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/random/RNGManager.h>

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
      
      corsika::setup::Trajectory step = fTracking.GetTrack(particle);
      const double total_inv_lambda = fProcesseList.GetTotalInverseInteractionLength(particle, step);
      
      // sample random exponential step length
      // ....
      static corsika::random::RNG& rmng =
	corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
      const double sample_step = rmng() / (double)rmng.max();
      const double next_step = -log(sample_step) / total_inv_lambda;

      const double min_step = fProcesseList.MinStepLength(particle, step);
      // convert next_step from grammage to length
      // Environment::GetDistance(step, next_step);
      // ....
      
      // update step with actual min(min_step, next_step)
      // ....
      
      /// here the particle is actually moved along the trajectory to new position:
      // std::visit(corsika::setup::ParticleUpdate<Particle>{particle}, step);
      particle.SetPosition(step.GetPosition(1));
      
      corsika::process::EProcessReturn status =
          fProcesseList.DoContinuous(particle, step, fStack);
      
      if (status == corsika::process::EProcessReturn::eParticleAbsorbed) {
        // fStack.Delete(particle); // TODO: check if this is really needed
      } else {

	// check if min_step < random_step  ->  skip discrete if rndm>min_step/random_step
	if (min_step<next_step) {
	  const double p_next = min_step/next_step;
	  const double sample_skip = rmng() / (double)rmng.max();
	  if (sample_skip>p_next) {
	    return;// corsika::process::EProcessReturn::eOk;
	  }
	}
	const double sample_process = rmng() / (double)rmng.max();
	double inv_lambda_count = 0;
	fProcesseList.SelectDiscrete(particle, fStack, total_inv_lambda,
				     sample_process, inv_lambda_count);
      }
    }

  private:
    Tracking& fTracking;
    ProcessList& fProcesseList;
    Stack& fStack;
  };

} // namespace corsika::cascade

#endif
