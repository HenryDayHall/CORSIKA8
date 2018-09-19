#ifndef _include_Cascade_h_
#define _include_Cascade_h_

#include <corsika/geometry/LineTrajectory.h> // to be removed
#include <corsika/geometry/Point.h>          // to be removed
#include <corsika/units/PhysicalUnits.h>

using namespace corsika::units;

namespace corsika::cascade {

  template <typename Trajectory, typename ProcessList, typename Stack>
  class Cascade {

    typedef typename Stack::ParticleType Particle;

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
          //Particle& p = *fStack.GetNextParticle();
 	  EnergyType Emin;
	  typename Stack::StackIterator pMin(fStack, 0);
	  bool first = true;
	  for (typename Stack::StackIterator ip = fStack.begin(); ip!=fStack.end(); ++ip) 
	    {
	      if (first || ip.GetEnergy()<Emin) {
		first = false;
		pMin = ip;
		Emin = pMin.GetEnergy();
	      }
	    }
	  
          Step(pMin);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations(); //
      }
    }

    void Step(Particle& particle) {
      double nextStep = fProcesseList.MinStepLength(particle);
      corsika::geometry::CoordinateSystem root;
      Trajectory trajectory(
          corsika::geometry::Point(root, {0_m, 0_m, 0_m}),
          corsika::geometry::Vector<corsika::units::SpeedType::dimension_type>(
              root, 0 * 1_m / second, 0 * 1_m / second, 1 * 1_m / second));
      fProcesseList.DoContinuous(particle, trajectory, fStack);
      // if (particle.IsMarkedToBeDeleted())
      {
        // std::cout << "DELETET THISSKSKJD!" << std::endl;
        // fStack.Delete(particle);
      }
      fProcesseList.DoDiscrete(particle, fStack);
    }

  private:
    Stack& fStack;
    ProcessList& fProcesseList;
  };

} // namespace corsika::cascade

#endif
