Howto create new physics modules
================================

There are different types of physics modules, which you can add to the CORSIKA~8 physics process 
sequence. Modules can act on particles, secondaries or the entire stack. Modules can create new particles 
or they can modify or delete particles. They can also produce output. 

Types of different modules are explained in ::modules

When creating new modules, we suggest to stick as close as possible to the default CORSIKA~8 coding guidelines 
and code structure. This makes code review and sharing with others not more complicated than needed. 

When your modules creates output, use the available CORSIKA~8 output machinery. This is not explained here. 

Let's consider the case of an "InteractionProcess" which will remove the projectile particle and create 
secondary particles on the stack instead. It also has a cross section in order to evaulate the probability 
with respect to other InteractionProcesses. Create a header file "SimpleProcess.hpp", which is conceptually
based to resemble (roughly) a Matthews-Heitler model:

.. code-block:: c++

   #pragma once

   #include <corsika/framework/core/ParticleProperties.hpp>
   #include <corsika/framework/core/PhysicalConstants.hpp>
   #include <corsika/framework/core/PhysicalUnits.hpp>
   #include <corsika/framework/process/InteractionProcess.hpp>
   #include <corsika/framework/random/RNGManager.hpp>

   namespace corsika::simple_process {

   class SimpleProcess : public corsika::InteractionProcess<SimpleProcess> {
      
   public:
      SimpleProcess();

      template <typename TParticle>
      GrammageType getInteractionLength(TParticle const&) const;

      template <typename TSecondaryView>
      void doInteraction(TSecondaryView& view) const;    

   private:
      // the random number stream
      corsika::default_prng_type& rng_ =
         corsika::RNGManager::getInstance().getRandomStream("simple_process");
      // the logger


   };

   } 

   #include <SimpleProcess.inl>


And the corresponding "SimpleProcess.inl" as:

.. code-block:: c++

   namespace corsika::simple_process
   {

   inline SimpleProcess::SimpleProcess() {}

   template <typename TParticle>
   inline GrammageType SimpleProcess::getInteractionLength(
         TParticle const &particle) const
   {
      // this process only applies to hadrons above Ekin=100GeV 
      if (is_hadron(particle.getPID() && particle.getKineticEnergy()>100_GeV) {
        return 2_g/square(1_cm); 
      }
      // otherwise its cross section is 0
      return std::numeric_limits<double>::infitiny * 1_g/square(1_cm);
   }

   template <typename TSecondaryView>
   inline void SimpleProcess::doInteraction(TSecondaryView &view) const
   {
      // the actual projectile particle, which will disappear after the call to doInteraction finished.
      auto projectile = view.getProjectile();
      int const nMult = 15;
      auto pionEnergy = projectile.getEnergy() / nMult;

      auto const pOrig = proton.getPosition();
      auto const tOrig = proton.getTime();
      auto const &cs = projectileMomentum.getCoordinateSystem();

      for (int iMult=0; iMult<nMult; ++iMult) {

         proton.addSecondary(std::make_tuple(Code::PiPlus,
            pionEnergy, 
            projectileMomentum.normalized() * sqrt(square(pionEnergy) + square(PiPlus::mass)),
            pOrig,
            tOrig));

      }
   }

   }

The SimpleProcess is not necessarily useful for anything and its sole purpuse is to illustrate the mechanism to 
create your process. 

You can then include such a process in your programm (e.g. in vertical_EAS) with `#include <SimpleProcess.hpp>` 
and just initialize it `simple_process::SimpleProcess simple; ` as your wish and add it to the physiscs sequence 
e.g. via `auto extended_sequence = make_sequence(old_sequence, simple);`
