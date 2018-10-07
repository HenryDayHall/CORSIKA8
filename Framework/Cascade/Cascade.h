
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Cascade_h_
#define _include_Cascade_h_

#include <corsika/geometry/LineTrajectory.h> // to be removed. for dummy trajectory only
#include <corsika/geometry/Point.h>          // to be removed. for dummy trajectory only
#include <corsika/process/ProcessReturn.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika::units::si;

namespace corsika::cascade {

  template <typename ProcessList, typename Stack> //, typename Trajectory>
  class Cascade {

    typedef typename Stack::ParticleType Particle;

    Cascade() = delete;

  public:
    Cascade(ProcessList& pl, Stack& stack)
        : fProcesseList(pl)
        , fStack(stack) {}

    void Init() {
      fStack.Init();
      fProcesseList.Init();
    }

    void Run() {
      while (!fStack.IsEmpty()) {
        while (!fStack.IsEmpty()) {
          Particle& pNext = *fStack.GetNextParticle();
          /*
            EnergyType Emin;
            typename Stack::StackIterator pNext(fStack, 0);
            bool first = true;
            for (typename Stack::StackIterator ip = fStack.begin(); ip != fStack.end();
            ++ip) {
            if (first || ip.GetEnergy() < Emin) {
            first = false;
            pNext = ip;
            Emin = pMin.GetEnergy();
            }
            }
          */
          Step(pNext);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations(); //
      }
    }
    
    void Step(Particle& particle) {
      [[maybe_unused]] double nextStep = fProcesseList.MinStepLength(particle);
      // corsika::utls::ignore(nextStep);
      corsika::geometry::CoordinateSystem root;
      corsika::geometry::LineTrajectory
          trajectory( // trajectory is not yet used. this is a dummy.
              corsika::geometry::Point(root, {0_m, 0_m, 0_m}),
              corsika::geometry::Vector<corsika::units::si::SpeedType::dimension_type>(
                  root, 0 * 1_m / second, 0 * 1_m / second, 1 * 1_m / second));
      corsika::process::EProcessReturn status =
          fProcesseList.DoContinuous(particle, trajectory, fStack);
      if (status == corsika::process::EProcessReturn::eParticleAbsorbed) {
        fStack.Delete(particle);
      } else {
        fProcesseList.DoDiscrete(particle, fStack);
      }
    }
    
  private:
    ProcessList& fProcesseList;
    Stack& fStack;
  };

} // namespace corsika::cascade

#endif
