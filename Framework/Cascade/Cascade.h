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
#include <corsika/random/ExponentialDistribution.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/stack/SecondaryView.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupTrajectory.h>

/*  see Issue 161, we need to include SetupStack only because we need
    to globally define StackView. This is clearly not nice and should
    be changed, when possible. It might be that StackView needs to be
    templated in Cascade, but this would be even worse... so we don't
    do that until it is really needed.
 */
#include <corsika/setup/SetupStack.h>

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>

#include <boost/type_index.hpp>
using boost::typeindex::type_id_with_cvr;

/**
 * The cascade namespace assembles all objects needed to simulate full particles cascades.
 */

namespace corsika::cascade {

  /**
   * \class Cascade
   *
   * The Cascade class is constructed from template arguments making
   * it very versatile. Via the template arguments physics models are
   * plugged into the cascade simulation.
   *
   * <b>TTracking</b> must be a class according to the
   * TrackingInterface providing the functions:
   * <code>auto GetTrack(Particle const& p)</auto>,
   * with the return type <code>geometry::Trajectory<corsika::geometry::Line>
   * </code>
   *
   * <b>TProcessList</b> must be a ProcessSequence.   *
   * <b>Stack</b> is the storage object for particle data, i.e. with
   * Particle class type <code>Stack::ParticleType</code>
   *
   *
   */

  template <typename TTracking, typename TProcessList, typename TStack,
            /*
              TStackView is needed as template parameter because of issue 161 and the
              inability of clang to understand "MakeView" so far.
             */
            typename TStackView = corsika::setup::StackView>
  class Cascade {
    using Particle = typename TStack::ParticleType;
    using VolumeTreeNode =
        std::remove_pointer_t<decltype(((Particle*)nullptr)->GetNode())>;
    using MediumInterface = typename VolumeTreeNode::IModelProperties;

    // we only want fully configured objects
    Cascade() = delete;

  public:
    /**
     * Cascade class cannot be default constructed, but needs a valid
     * list of physics processes for configuration at construct time.
     */
    Cascade(corsika::environment::Environment<MediumInterface> const& env, TTracking& tr,
            TProcessList& pl, TStack& stack)
        : fEnvironment(env)
        , fTracking(tr)
        , fProcessSequence(pl)
        , fStack(stack) {}

    /**
     * The Init function is called before the actual cascade simulations.
     * All components of the Cascade simulation must be configured here.
     */
    void Init() {
      fProcessSequence.Init();
      fStack.Init();
    }

    /**
     * set the nodes for all particles on the stack according to their numerical
     * position
     */
    void SetNodes() {
      std::for_each(fStack.begin(), fStack.end(), [&](auto& p) {
        auto const* numericalNode =
            fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
        p.SetNode(numericalNode);
      });
    }

    /**
     * The Run function is the main simulation loop, which processes
     * particles from the Stack until the Stack is empty.
     */
    void Run() {
      SetNodes();

      while (!fStack.IsEmpty()) {
        while (!fStack.IsEmpty()) {
          auto pNext = fStack.GetNextParticle();
          Step(pNext);
          fProcessSequence.DoStack(fStack);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations();
      }
    }

  private:
    /**
     * The Step function is executed for each particle from the
     * stack. It will calcualte geometric transport of the particles,
     * and apply continuous and stochastic processes to it, which may
     * lead to energy losses, scattering, absorption, decays and the
     * production of secondary particles.
     *
     * New particles produced in one step are subject to further
     * processing, e.g. thinning, etc.
     */
    void Step(Particle& vParticle) {
      using namespace corsika;
      using namespace corsika::units::si;

      // determine geometric tracking
      auto [step, geomMaxLength, nextVol] = fTracking.GetTrack(vParticle);
      [[maybe_unused]] auto const& dummy_nextVol = nextVol;

      // determine combined total interaction length (inverse)
      InverseGrammageType const total_inv_lambda =
          fProcessSequence.GetTotalInverseInteractionLength(vParticle);

      // sample random exponential step length in grammage
      corsika::random::ExponentialDistribution expDist(1 / total_inv_lambda);
      GrammageType const next_interact = expDist(fRNG);

      std::cout << "total_inv_lambda=" << total_inv_lambda
                << ", next_interact=" << next_interact << std::endl;

      auto const* currentLogicalNode = vParticle.GetNode();

      // assert that particle stays outside void Universe if it has no
      // model properties set
      assert(currentLogicalNode != &*fEnvironment.GetUniverse() ||
             fEnvironment.GetUniverse()->HasModelProperties());

      // convert next_step from grammage to length
      LengthType const distance_interact =
          currentLogicalNode->GetModelProperties().ArclengthFromGrammage(step,
                                                                         next_interact);

      // determine the maximum geometric step length
      LengthType const distance_max = fProcessSequence.MaxStepLength(vParticle, step);
      std::cout << "distance_max=" << distance_max << std::endl;

      // determine combined total inverse decay time
      InverseTimeType const total_inv_lifetime =
          fProcessSequence.GetTotalInverseLifetime(vParticle);

      // sample random exponential decay time
      corsika::random::ExponentialDistribution expDistDecay(1 / total_inv_lifetime);
      TimeType const next_decay = expDistDecay(fRNG);
      std::cout << "total_inv_lifetime=" << total_inv_lifetime
                << ", next_decay=" << next_decay << std::endl;

      // convert next_decay from time to length [m]
      LengthType const distance_decay = next_decay * vParticle.GetMomentum().norm() /
                                        vParticle.GetEnergy() * units::constants::c;

      // take minimum of geometry, interaction, decay for next step
      auto const min_distance =
          std::min({distance_interact, distance_decay, distance_max, geomMaxLength});

      std::cout << " move particle by : " << min_distance << std::endl;

      // here the particle is actually moved along the trajectory to new position:
      // std::visit(setup::ParticleUpdate<Particle>{vParticle}, step);
      vParticle.SetPosition(step.PositionFromArclength(min_distance));
      // .... also update time, momentum, direction, ...
      vParticle.SetTime(vParticle.GetTime() + min_distance / units::constants::c);

      step.LimitEndTo(min_distance);

      // apply all continuous processes on particle + track
      process::EProcessReturn status = fProcessSequence.DoContinuous(vParticle, step);

      if (status == process::EProcessReturn::eParticleAbsorbed) {
        std::cout << "Cascade: delete absorbed particle " << vParticle.GetPID() << " "
                  << vParticle.GetEnergy() / 1_GeV << "GeV" << std::endl;
        vParticle.Delete();
        return;
      }

      std::cout << "sth. happening before geometric limit ? "
                << ((min_distance < geomMaxLength) ? "yes" : "no") << std::endl;

      if (min_distance < geomMaxLength) { // interaction to happen within geometric limit

        // check whether decay or interaction limits this step the
        // outcome of decay or interaction MAY be a) new particles in
        // secondaries, b) the projectile particle deleted (or
        // changed)

        TStackView secondaries(vParticle);

        if (min_distance != distance_max) {
          /*
            Create SecondaryView object on Stack. The data container
            remains untouched and identical, and 'projectil' is identical
            to 'vParticle' above this line. However,
            projectil.AddSecondaries populate the SecondaryView, which can
            then be used afterwards for further processing. Thus: it is
            important to use projectle (and not vParticle) for Interaction,
            and Decay!
          */

          [[maybe_unused]] auto projectile = secondaries.GetProjectile();

          if (min_distance == distance_interact) {
            std::cout << "collide" << std::endl;

            InverseGrammageType const current_inv_length =
                fProcessSequence.GetTotalInverseInteractionLength(vParticle);

            random::UniformRealDistribution<InverseGrammageType> uniDist(
                current_inv_length);
            const auto sample_process = uniDist(fRNG);
            InverseGrammageType inv_lambda_count = 0. * meter * meter / gram;
            fProcessSequence.SelectInteraction(vParticle, projectile, sample_process,
                                               inv_lambda_count);
          } else {
            assert(min_distance == distance_decay);
            std::cout << "decay" << std::endl;
            InverseTimeType const actual_decay_time =
                fProcessSequence.GetTotalInverseLifetime(vParticle);

            random::UniformRealDistribution<InverseTimeType> uniDist(actual_decay_time);
            const auto sample_process = uniDist(fRNG);
            InverseTimeType inv_decay_count = 0 / second;
            fProcessSequence.SelectDecay(vParticle, projectile, sample_process,
                                         inv_decay_count);
            // make sure particle actually did decay if it should have done so
            if (secondaries.GetSize() == 1 &&
                projectile.GetPID() == secondaries.GetNextParticle().GetPID())
              throw std::runtime_error("Cascade::Step: Particle decays into itself!");
          }

          fProcessSequence.DoSecondaries(secondaries);
          vParticle.Delete(); // todo: this should be reviewed. Where
                              // exactly are particles best deleted, and
                              // where they should NOT be
                              // deleted... maybe Delete function should
                              // be "protected" and not accessible to physics

        } else { // step-length limitation within volume

          std::cout << "step-length limitation" << std::endl;
          fProcessSequence.DoSecondaries(secondaries);
        }

        [[maybe_unused]] auto const assertion = [&] {
          auto const* numericalNodeAfterStep =
              fEnvironment.GetUniverse()->GetContainingNode(vParticle.GetPosition());
          return numericalNodeAfterStep == currentLogicalNode;
        };

        assert(assertion()); // numerical and logical nodes don't match
      } else {               // boundary crossing, step is limited by volume boundary
        std::cout << "boundary crossing! next node = " << nextVol << std::endl;
        vParticle.SetNode(nextVol);
        // DoBoundary may delete the particle (or not)
        fProcessSequence.DoBoundaryCrossing(vParticle, *currentLogicalNode, *nextVol);
      }
    }

  private:
    corsika::environment::Environment<MediumInterface> const& fEnvironment;
    TTracking& fTracking;
    TProcessList& fProcessSequence;
    TStack& fStack;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");
  }; // namespace corsika::cascade

} // namespace corsika::cascade

#endif
