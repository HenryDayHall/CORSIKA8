/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>
#include <corsika/logging/Logging.h>
#include <corsika/process/ProcessReturn.h>
#include <corsika/random/ExponentialDistribution.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/stack/SecondaryView.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/stack/history/EventType.hpp>
#include <corsika/stack/history/HistorySecondaryProducer.hpp>

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
#include <limits>

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
              TStackView is needed as explicit template parameter because
              of issue 161 and the
              inability of clang to understand "stack::MakeView" so far.
             */
            typename TStackView = corsika::setup::StackView>
  class Cascade {
    using Particle = typename TStack::ParticleType;
    using VolumeTreeNode =
        std::remove_pointer_t<decltype(((Particle*)nullptr)->GetNode())>;
    using MediumInterface = typename VolumeTreeNode::IModelProperties;

  private:
    // Data members
    corsika::environment::Environment<MediumInterface> const& environment_;
    TTracking& tracking_;
    TProcessList& process_sequence_;
    TStack& stack_;
    corsika::random::RNG& rng_ =
        corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");
    unsigned int count_ = 0;

  private:
    // we only want fully configured objects
    Cascade() = delete;

  public:
    /**
     * Cascade class cannot be default constructed, but needs a valid
     * list of physics processes for configuration at construct time.
     */
    Cascade(corsika::environment::Environment<MediumInterface> const& env, TTracking& tr,
            TProcessList& pl, TStack& stack)
        : environment_(env)
        , tracking_(tr)
        , process_sequence_(pl)
        , stack_(stack)
        , count_(0) {
      C8LOG_INFO(c8_ascii_);
      if constexpr (TStackView::has_event) {
        C8LOG_INFO(" - With full cascade HISTORY.");
      }
    }

    /**
     * The Run function is the main simulation loop, which processes
     * particles from the Stack until the Stack is empty.
     */
    void Run() {
      setNodes();

      while (!stack_.IsEmpty()) {
        while (!stack_.IsEmpty()) {
          C8LOG_TRACE("Stack: {}", stack_.as_string());
          count_++;
          auto pNext = stack_.GetNextParticle();
          C8LOG_DEBUG(
              "============== next particle : count={}, pid={}, "
              ", stack entries={}"
              ", stack deleted={}",
              count_, pNext.GetPID(), stack_.getEntries(), stack_.getDeleted());
          Step(pNext);
          process_sequence_.DoStack(stack_);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations();
      }
    }

    /**
     * Force an interaction of the top particle of the stack at its current position.
     * Note that SetNodes() or an equivalent procedure needs to be called first if you
     * want to call forceInteraction() for the primary interaction.
     */
    void forceInteraction() {
      C8LOG_DEBUG("forced interaction!");
      setNodes();
      auto vParticle = stack_.GetNextParticle();
      TStackView secondaries(vParticle);
      interaction(secondaries);
      process_sequence_.DoSecondaries(secondaries);
      vParticle.Delete(); // primary particle has interacted and is gone
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
      auto [step, geomMaxLength, nextVol] = tracking_.GetTrack(vParticle);
      [[maybe_unused]] auto const& dummy_nextVol = nextVol;

      // determine combined total interaction length (inverse)
      InverseGrammageType const total_inv_lambda =
          process_sequence_.GetInverseInteractionLength(vParticle);

      // sample random exponential step length in grammage
      corsika::random::ExponentialDistribution expDist(1 / total_inv_lambda);
      GrammageType const next_interact = expDist(rng_);

      C8LOG_DEBUG(
          "total_lambda={} g/cm2, "
          ", next_interact={} g/cm2",
          double((1. / total_inv_lambda) / 1_g * 1_cm * 1_cm),
          double(next_interact / 1_g * 1_cm * 1_cm));

      auto const* currentLogicalNode = vParticle.GetNode();

      // assert that particle stays outside void Universe if it has no
      // model properties set
      assert(currentLogicalNode != &*environment_.GetUniverse() ||
             environment_.GetUniverse()->HasModelProperties());

      // convert next_step from grammage to length
      LengthType const distance_interact =
          currentLogicalNode->GetModelProperties().ArclengthFromGrammage(step,
                                                                         next_interact);

      // determine the maximum geometric step length from continuous processes
      LengthType const distance_max = process_sequence_.MaxStepLength(vParticle, step);
      C8LOG_DEBUG("distance_max={} m", distance_max / 1_m);

      // determine combined total inverse decay time
      InverseTimeType const total_inv_lifetime =
          process_sequence_.GetInverseLifetime(vParticle);

      // sample random exponential decay time
      corsika::random::ExponentialDistribution expDistDecay(1 / total_inv_lifetime);
      TimeType const next_decay = expDistDecay(rng_);
      C8LOG_DEBUG(
          "total_lifetime={} s"
          ", next_decay={} s",
          (1 / total_inv_lifetime) / 1_s, next_decay / 1_s);

      // convert next_decay from time to length [m]
      LengthType const distance_decay = next_decay * vParticle.GetMomentum().norm() /
                                        vParticle.GetEnergy() * units::constants::c;
                                    
      // determine geometric tracking
      auto [stepWithoutB, stepWithB, geomMaxLength, nextVol] = fTracking.GetTrack(vParticle);
      [[maybe_unused]] auto const& dummy_nextVol = nextVol;
      
      // convert next_step from grammage to length
      LengthType const distance_interact =
          currentLogicalNode->GetModelProperties().ArclengthFromGrammage(stepWithB,
                                                                         next_interact);
      
      // determine the maximum geometric step length
      LengthType const distance_max = fProcessSequence.MaxStepLength(vParticle, stepWithoutB);
      std::cout << "distance_max=" << distance_max << std::endl;

      // take minimum of geometry, interaction, decay for next step
      std::cout << "Interaction: " << distance_interact << std::endl;
      std::cout << "Decay: " << distance_decay << std::endl;
      std::cout << "ObsPlane: " << distance_max << std::endl;
      std::cout << "Transition: " << geomMaxLength << std::endl;
      auto const min_distance = std::min(
          {distance_interact, distance_decay, distance_max, geomMaxLength});

      C8LOG_DEBUG("transport particle by : {} m", min_distance / 1_m);

      // determine displacement by the magnetic field
	    auto const* currentLogicalVolumeNode = vParticle.GetNode();
      auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(vParticle.GetPosition());
      geometry::Vector<SpeedType::dimension_type> velocity = vParticle.GetMomentum() / vParticle.GetEnergy() *
                                                              corsika::units::constants::c;
  		geometry::Vector<dimensionless_d> const directionBefore = velocity.normalized();
      int chargeNumber;
      if (corsika::particles::IsNucleus(vParticle.GetPID())) {
        chargeNumber = vParticle.GetNuclearZ();
      } else {
     	  chargeNumber = corsika::particles::GetChargeNumber(vParticle.GetPID());
      }
      auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / 
              (velocity.GetNorm() * vParticle.GetEnergy() * 1_V);
		  
  		// First Movement
  		// assuming magnetic field does not change during movement
  		auto position = vParticle.GetPosition() + directionBefore * min_distance / 2;
  		// Change of direction by magnetic field
  		geometry::Vector<dimensionless_d> const directionAfter = directionBefore + directionBefore.cross(magneticfield) *
                                                              min_distance * k; 
  		// Second Movement
  		position = position + directionAfter * min_distance / 2;
      auto distance = position - vParticle.GetPosition();
      //distance.norm() != min_distance if q != 0
      //small error can be neglected
      if (distance.norm() != 0_m) {
      velocity = distance.normalized() * velocity.norm();
      } //no velocity update for very small steps
      
      // here the particle is actually moved along the trajectory to new position:
      // std::visit(setup::ParticleUpdate<Particle>{vParticle}, step);
      vParticle.SetMomentum(directionAfter.normalized() * vParticle.GetMomentum().norm());
      geometry::Line line(vParticle.GetPosition(), velocity);
      geometry::Trajectory<geometry::Line> stepNew(line, distance.norm() / velocity.norm());
      vParticle.SetPosition(position);
      vParticle.SetTime(vParticle.GetTime() + distance.norm() / units::constants::c);
      std::cout << "New Position: " << vParticle.GetPosition().GetCoordinates() << std::endl;

      // apply all continuous processes on particle + track
      process::EProcessReturn status = fProcessSequence.DoContinuous(vParticle, stepNew);

      if (status == process::EProcessReturn::eParticleAbsorbed) {
        C8LOG_DEBUG("Cascade: delete absorbed particle PID={} E={} GeV",
                    vParticle.GetPID(), vParticle.GetEnergy() / 1_GeV);
        if (!vParticle.isDeleted()) vParticle.Delete();
        return;
      }

      C8LOG_DEBUG("sth. happening before geometric limit ? {}",
                  ((min_distance < geomMaxLength) ? "yes" : "no"));

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
            important to use projectle/view (and not vParticle) for Interaction,
            and Decay!
          */

          [[maybe_unused]] auto projectile = secondaries.GetProjectile();

          if (min_distance == distance_interact) {
            interaction(secondaries);
          } else {
            assert(min_distance == distance_decay);
            decay(secondaries);
            // make sure particle actually did decay if it should have done so
            if (secondaries.getSize() == 1 &&
                projectile.GetPID() == secondaries.GetNextParticle().GetPID())
              throw std::runtime_error(
                  fmt::format("Cascade: {} decayed into itself!",
                              particles::GetName(projectile.GetPID())));
          }

          process_sequence_.DoSecondaries(secondaries);
          vParticle.Delete();

        } else { // step-length limitation within volume
          C8LOG_DEBUG("step-length limitation");
          // no extra physics happens here. just proceed to next step.
        }

        [[maybe_unused]] auto const assertion = [&] {
          auto const* numericalNodeAfterStep =
              environment_.GetUniverse()->GetContainingNode(vParticle.GetPosition());
          C8LOG_TRACE("Geometry check: numericalNodeAfterStep={} currentLogicalNode={}",
                      fmt::ptr(numericalNodeAfterStep), fmt::ptr(currentLogicalNode));
          return numericalNodeAfterStep == currentLogicalNode;
        };

        assert(assertion()); // numerical and logical nodes don't match
      } else {               // boundary crossing, step is limited by volume boundary
        vParticle.SetNode(nextVol);
        /*
          DoBoundary may delete the particle (or not)

          caveat: any changes to vParticle, or even the production
          of new secondaries is currently not passed to ParticleCut,
          thus, particles outside the desired phase space may be produced.

          todo: this must be fixed.
        */
        process_sequence_.DoBoundaryCrossing(vParticle, *currentLogicalNode, *nextVol);
      }
    }

    process::EProcessReturn decay(TStackView& view) {
      C8LOG_DEBUG("decay");
      units::si::InverseTimeType const actual_decay_time =
          process_sequence_.GetInverseLifetime(view.parent());

      random::UniformRealDistribution<units::si::InverseTimeType> uniDist(
          actual_decay_time);
      const auto sample_process = uniDist(rng_);
      auto const returnCode = process_sequence_.SelectDecay(view, sample_process);
      if (returnCode != process::EProcessReturn::eDecayed) {
        C8LOG_WARN("Particle did not decay!");
      }
      SetEventType(view, history::EventType::Decay);
      return returnCode;
    }

    process::EProcessReturn interaction(TStackView& view) {
      C8LOG_DEBUG("collide");

      units::si::InverseGrammageType const current_inv_length =
          process_sequence_.GetInverseInteractionLength(view.parent());

      random::UniformRealDistribution<units::si::InverseGrammageType> uniDist(
          current_inv_length);
      const auto sample_process = uniDist(rng_);
      auto const returnCode = process_sequence_.SelectInteraction(view, sample_process);
      if (returnCode != process::EProcessReturn::eInteracted) {
        C8LOG_WARN("Particle did not interace!");
      }
      SetEventType(view, history::EventType::Interaction);
      return returnCode;
    }

    /**
     * set the nodes for all particles on the stack according to their numerical
     * position
     */
    void setNodes() {
      std::for_each(stack_.begin(), stack_.end(), [&](auto& p) {
        auto const* numericalNode =
            environment_.GetUniverse()->GetContainingNode(p.GetPosition());
        p.SetNode(numericalNode);
      });
    }

    void SetEventType(TStackView& view, [[maybe_unused]] history::EventType eventType) {
      if constexpr (TStackView::has_event) {
        for (auto&& sec : view) { sec.GetEvent()->setEventType(eventType); }
      }
    }

    // but this here temporarily. Should go into dedicated file later:
    const char* c8_ascii_ =
        R"V0G0N(
  ,ad8888ba,     ,ad8888ba,    88888888ba    ad88888ba   88  88      a8P          db              ad88888ba   
 d8"'    `"8b   d8"'    `"8b   88      "8b  d8"     "8b  88  88    ,88'          d88b            d8"     "8b  
d8'            d8'        `8b  88      ,8P  Y8,          88  88  ,88"           d8'`8b           Y8a     a8P  
88             88          88  88aaaaaa8P'  `Y8aaaaa,    88  88,d88'           d8'  `8b           "Y8aaa8P"   
88             88          88  88""""88'      `"""""8b,  88  8888"88,         d8YaaaaY8b          ,d8"""8b,   
Y8,            Y8,        ,8P  88    `8b            `8b  88  88P   Y8b       d8""""""""8b        d8"     "8b  
 Y8a.    .a8P   Y8a.    .a8P   88     `8b   Y8a     a8P  88  88     "88,    d8'        `8b       Y8a     a8P  
  `"Y8888Y"'     `"Y8888Y"'    88      `8b   "Y88888P"   88  88       Y8b  d8'          `8b       "Y88888P"
	)V0G0N";
  };

} // namespace corsika::cascade
