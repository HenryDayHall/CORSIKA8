
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

#include <type_traits>

#include <corsika/setup/SetupTrajectory.h>

namespace corsika::cascade {

  template <typename Tracking, typename ProcessList, typename Stack>
  class Cascade {

    typedef typename Stack::ParticleType Particle;

    Cascade() = delete;

  public:
    Cascade(Tracking& tr, ProcessList& pl, Stack& stack)
        : fTracking(tr)
        , fProcesseList(pl)
        , fStack(stack) {
      // static_assert(std::is_member_function_pointer<decltype(&ProcessList::DoDiscrete)>::value,
      //"ProcessList has not function DoDiscrete.");
      // static_assert(std::is_member_function_pointer<decltype(&ProcessList::DoContinuous)>::value,
      //	    "ProcessList has not function DoContinuous.");
    }

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
        // DoCascadeEquations(); //
      }
    }

    void Step(Particle& particle) {
      corsika::setup::Trajectory step = fTracking.GetTrack(particle);
      fProcesseList.MinStepLength(particle, step);

      /// here the particle is actually moved along the trajectory to new position:
      std::visit(corsika::setup::ParticleUpdate<Particle>{particle}, step);

      corsika::process::EProcessReturn status =
          fProcesseList.DoContinuous(particle, step, fStack);
      if (status == corsika::process::EProcessReturn::eParticleAbsorbed) {
        fStack.Delete(particle); // TODO: check if this is really needed
      } else {
        fProcesseList.DoDiscrete(particle, fStack);
      }
    }

  private:
    Tracking& fTracking;
    ProcessList& fProcesseList;
    Stack& fStack;
  };

} // namespace corsika::cascade

#endif
