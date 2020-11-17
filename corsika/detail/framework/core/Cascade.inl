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

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/framework/random/ExponentialDistribution.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>
#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/media/Environment.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>


namespace corsika {

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::Init() {
    fProcessSequence.Init();
    fStack.Init();
  }

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::SetNodes() {
    std::for_each(fStack.begin(), fStack.end(), [&](auto& p) {
      auto const* numericalNode =
          fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
      p.SetNode(numericalNode);
    });
  }

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::Run() {
    SetNodes();

    while (!fStack.IsEmpty()) {
      while (!fStack.IsEmpty()) {
        auto pNext = fStack.GetNextParticle();
        std::cout << "========= next: " << pNext.GetPID() << std::endl;
        Step(pNext);
        std::cout << "========= stack ============" << std::endl;
        fProcessSequence.DoStack(fStack);
      }
      // do cascade equations, which can put new particles on Stack,
      // thus, the double loop
      // DoCascadeEquations();
    }
  }

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::forceInteraction() {
    std::cout << "forced interaction!" << std::endl;
    auto vParticle = fStack.GetNextParticle();
    TStackView secondaries(vParticle);
    auto projectile = secondaries.GetProjectile();
    interaction(vParticle, projectile);
    fProcessSequence.DoSecondaries(secondaries);
    vParticle.Delete(); // todo: this should be reviewed, see below
  }

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::Step(Particle& vParticle) {

    // determine geometric tracking
    auto [step, geomMaxLength, nextVol] = fTracking.GetTrack(vParticle);
    [[maybe_unused]] auto const& dummy_nextVol = nextVol;

    // determine combined total interaction length (inverse)
    InverseGrammageType const total_inv_lambda =
        fProcessSequence.GetTotalInverseInteractionLength(vParticle);

    // sample random exponential step length in grammage
    corsika::ExponentialDistribution expDist(1 / total_inv_lambda);
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
        currentLogicalNode->GetModelProperties().arclengthFromGrammage(step,
                                                                       next_interact);

    // determine the maximum geometric step length
    LengthType const distance_max = fProcessSequence.MaxStepLength(vParticle, step);
    std::cout << "distance_max=" << distance_max << std::endl;

    // determine combined total inverse decay time
    InverseTimeType const total_inv_lifetime =
        fProcessSequence.GetTotalInverseLifetime(vParticle);

    // sample random exponential decay time
    corsika::ExponentialDistribution expDistDecay(1 / total_inv_lifetime);
    TimeType const next_decay = expDistDecay(fRNG);
    std::cout << "total_inv_lifetime=" << total_inv_lifetime
              << ", next_decay=" << next_decay << std::endl;

    // convert next_decay from time to length [m]
    LengthType const distance_decay = next_decay * vParticle.GetMomentum().norm() /
                                      vParticle.GetEnergy() * constants::c;

    // take minimum of geometry, interaction, decay for next step
    auto const min_distance =
        std::min({distance_interact, distance_decay, distance_max, geomMaxLength});

    std::cout << " move particle by : " << min_distance << std::endl;

    // here the particle is actually moved along the trajectory to new position:
    // std::visit(setup::ParticleUpdate<particle_type>{vParticle}, step);
    vParticle.SetPosition(step.PositionFromArclength(min_distance));
    // .... also update time, momentum, direction, ...
    vParticle.SetTime(vParticle.GetTime() + min_distance / constants::c);

    step.LimitEndTo(min_distance);

    // apply all continuous processes on particle + track
    corsika::EProcessReturn status = fProcessSequence.DoContinuous(vParticle, step);

    if (status == corsika::EProcessReturn::eParticleAbsorbed) {
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
          interaction(vParticle, projectile);
        } else {
          assert(min_distance == distance_decay);
          decay(vParticle, projectile);
          // make sure particle actually did decay if it should have done so
          if (secondaries.GetSize() == 1 &&
              projectile.GetPID() == secondaries.GetNextParticle().GetPID())
            throw std::runtime_error("Cascade::Step: particle_type decays into itself!");
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

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  auto Cascade<TTracking, TProcessList, TStack, TStackView>::decay(
      Particle& particle,
      decltype(std::declval<TStackView>().GetProjectile()) projectile) {
    std::cout << "decay" << std::endl;
    InverseTimeType const actual_decay_time =
        fProcessSequence.GetTotalInverseLifetime(particle);

    corsika::UniformRealDistribution<InverseTimeType> uniDist(actual_decay_time);
    const auto sample_process = uniDist(fRNG);
    InverseTimeType inv_decay_count = InverseTimeType::zero();
    return fProcessSequence.SelectDecay(particle, projectile, sample_process,
                                        inv_decay_count);
  }

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  auto Cascade<TTracking, TProcessList, TStack, TStackView>::interaction(
      Particle& particle,
      decltype(std::declval<TStackView>().GetProjectile()) projectile) {
    std::cout << "collide" << std::endl;

    InverseGrammageType const current_inv_length =
        fProcessSequence.GetTotalInverseInteractionLength(particle);

    corsika::UniformRealDistribution<InverseGrammageType> uniDist(current_inv_length);
    const auto sample_process = uniDist(fRNG);
    auto inv_lambda_count = InverseGrammageType::zero();
    return fProcessSequence.SelectInteraction(particle, projectile, sample_process,
                                              inv_lambda_count);
  }

} // namespace corsika
