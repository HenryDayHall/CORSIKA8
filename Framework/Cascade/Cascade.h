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

#include <fstream>
#include <boost/histogram.hpp>
#include <boost/histogram/ostream.hpp>
#include <corsika/process/tracking_line/dump_bh.hpp>
using namespace boost::histogram;
static auto histL2 = make_histogram(axis::regular<>(100, 0, 60000, "L'"));
static auto histS2 = make_histogram(axis::regular<>(100, 0, 60000, "S"));
static auto histB2 = make_histogram(axis::regular<>(100, 0, 60000, "Bogenlänge"));
static auto histLlog2 = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Leap-Frog-ength L'"));
static auto histLlog2int = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Leap-Frog-ength L'"));
static auto histLlog2dec = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Leap-Frog-ength L'"));
static auto histLlog2max = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Leap-Frog-ength L'"));
static auto histLlog2geo = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Leap-Frog-ength L'"));
static auto histLlog2mag = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Leap-Frog-ength L'"));

static auto histSlog2 = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Direct Length S"));
static auto histBlog2 = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "Arc Length B"));
static auto histLB2 = make_histogram(axis::regular<>(100, 0, 0.01, "L - B"));
static auto histLS2 = make_histogram(axis::regular<>(100, 0, 0.01, "L - S"));
static auto histLBrel2 = make_histogram(axis::regular<double, axis::transform::log> (20,1e-11,1e-6,"L/B -1"));
static auto histLSrel2 = make_histogram(axis::regular<double, axis::transform::log>(20,1e-11,1e-6, "L/S -1"));
static auto histELSrel2 = make_histogram(axis::regular<double, axis::transform::log>(20,1e-11,1e-6, "L/S -1"),axis::regular<double, axis::transform::log>(20, 0.1, 1e4, "E / GeV"));
static auto histBS2 = make_histogram(axis::regular<>(100, 0, 0.01, "B - S"));
static auto histLp2 = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "L' für Protonen"));
static auto histLpi2 = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "L' für Pionen"));
static auto histLmu2 = make_histogram(axis::regular<double, axis::transform::log>(100, 1, 1e7, "L' für Myonen"));
//static auto histLe = make_histogram(axis::regular<>(100, 0, 60000, "L' für Elektronen"));
//static auto histLy = make_histogram(axis::regular<>(100, 0, 60000, "L' für Photonen"));

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

    ~Cascade(){
		  std::ofstream myfile;
          myfile.open ("histograms2.txt");
          myfile << histLp2 << std::endl;
          myfile << histLpi2 << std::endl;
          myfile << histLmu2 << std::endl;
          myfile << histL2 << std::endl;
          myfile.close(); 
		  
		  /*std::cout << histLBrel << std::endl;
		  std::cout << histLSrel << std::endl;*/

		  
		      std::ofstream file1("histL2.json");
          dump_bh(file1, histL2);
          file1.close();
          std::ofstream file2("histS2.json");
          dump_bh(file2, histS2);
          file2.close();
          std::ofstream file3("histB2.json");
          dump_bh(file3, histB2);
          file3.close();
          /*std::ofstream file4("histLB.json");
          dump_bh(file4, histLB);
          file4.close();
          std::ofstream file5("histLS.json");
          dump_bh(file5, histLS);
          file5.close();
          std::ofstream file6("histBS.json");
          dump_bh(file6, histBS);
          file6.close();
          std::ofstream file7("histLBrel.json");
          dump_bh(file7, histLBrel);
          file7.close();
          std::ofstream file8("histLSrel.json");
          dump_bh(file8, histLSrel);
          file8.close();
          
          std::ofstream file10("histELSrel.json");
          dump_bh(file10, histELSrel);
          file10.close();*/
          std::ofstream file11("histLmu2.json");
          dump_bh(file11, histLmu2);
          file11.close();
          std::ofstream file12("histLpi2.json");
          dump_bh(file12, histLpi2);
          file12.close();
          std::ofstream file13("histLp2.json");
          dump_bh(file13, histLp2);
          file13.close();
          std::ofstream file14("histLlog2.json");
          dump_bh(file14, histLlog2);
          file14.close();
          std::ofstream file15("histBlog2.json");
          dump_bh(file15, histBlog2);
          file15.close();
          std::ofstream file16("histSlog2.json");
          dump_bh(file16, histSlog2);
          file16.close();
          std::ofstream file17("histLlog2int.json");
          dump_bh(file17, histLlog2int);
          file17.close();
          std::ofstream file18("histLlog2dec.json");
          dump_bh(file18, histLlog2dec);
          file18.close();
          
          std::ofstream file20("histLlog2mag.json");
          dump_bh(file20, histLlog2mag);
          file20.close();
          std::ofstream file21("histLlog2geo.json");
          dump_bh(file21, histLlog2geo);
          file21.close();
          std::ofstream file22("histLlog2max.json");
          dump_bh(file22, histLlog2max);
          file22.close();
		  
		  };

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
      auto [stepWithoutB, stepWithB, geomMaxLength, magMaxLength, nextVol] = fTracking.GetTrack(vParticle);
      [[maybe_unused]] auto const& dummy_nextVol = nextVol;
      
      // convert next_step from grammage to length
      LengthType const distance_interact =
          currentLogicalNode->GetModelProperties().ArclengthFromGrammage(stepWithB,
                                                                         next_interact);
      
      // determine the maximum geometric step length
      LengthType const distance_max = fProcessSequence.MaxStepLength(vParticle, stepWithoutB);
      C8LOG_DEBUG("distance_max={} m", distance_max / 1_m);

      // take minimum of geometry, interaction, decay for next step
      auto min_distance = std::min(
          {distance_interact, distance_decay, distance_max, geomMaxLength, magMaxLength});

      C8LOG_DEBUG("transport particle by : {} m "
		  "Max Displacement after: {} m "
		  "Medium transition after: {} m "
		  "Decay after: {} m "
		  "Interaction after: {} m", 
		  min_distance/1_m, magMaxLength/1_m, geomMaxLength/1_m, distance_decay/1_m, distance_interact/1_m);

      // determine displacement by the magnetic field
      /*
      int chargeNumber;
	    if (corsika::particles::IsNucleus(vParticle.GetPID())) {
	      chargeNumber = vParticle.GetNuclearZ();
	    } else {
	      chargeNumber = corsika::particles::GetChargeNumber(vParticle.GetPID());
	    }
	    auto const* currentLogicalVolumeNode = vParticle.GetNode();
	    auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(vParticle.GetPosition());
	    auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / 
               ((vParticle.GetMomentum() / vParticle.GetEnergy() * 
               corsika::units::constants::c).norm() * vParticle.GetEnergy() * 1_V);
	    geometry::Vector<dimensionless_d> const directionBefore = vParticle.GetMomentum().normalized();
	    LengthType Steplength = min_distance;
	    if (chargeNumber != 0) {
		    Steplength = (-sqrt(2 * k * min_distance * (directionBefore.cross(magneticfield)).norm() + 1) - 1)
	                 / ((directionBefore.cross(magneticfield)).norm() * k);
        std::cout << "Steplength2 " << Steplength << std::endl;
        
	      Steplength = (sqrt(2 * k * min_distance * (directionBefore.cross(magneticfield)).norm() + 1) - 1)
	                 / ((directionBefore.cross(magneticfield)).norm() * k);
        std::cout << "Steplength1 " << Steplength << std::endl;
	      // not totally sure if it is always the positive solution
	    } 
	    */
	    // This formula has an error or doesnt work for specific conditions
	    // Steplength should not be min_distance
	    
      auto [position, direction, L2] = fTracking.MagneticStep(vParticle, min_distance);
      histL2(L2);
      histLlog2(L2);
      if (min_distance == distance_interact)
        histLlog2int(L2);
      if (min_distance == distance_decay)
        histLlog2dec(L2);
      if (min_distance == distance_max)
        histLlog2max(L2);
      if (min_distance == geomMaxLength)
        histLlog2geo(L2);
      if (min_distance == magMaxLength) {
        histLlog2mag(L2);
      }
      int pdg = static_cast<int>(particles::GetPDG(vParticle.GetPID()));
            if (abs(pdg) == 13)
              histLmu2(L2);
            if (abs(pdg) == 211 || abs(pdg) == 111)
              histLpi2(L2);
            if (abs(pdg) == 2212 || abs(pdg) == 2112)
              histLp2(L2);
      auto distance = position - vParticle.GetPosition();
      
      //Building Trajectory for Continuous processes
      geometry::Vector<SpeedType::dimension_type> velocity =
            vParticle.GetMomentum() / vParticle.GetEnergy() * corsika::units::constants::c;
      if (distance.norm() != 0_m) {
        velocity = distance.normalized() * velocity.norm();
      }
      geometry::Line line(vParticle.GetPosition(), velocity);
      geometry::Trajectory<geometry::Line> stepNew(line, distance.norm() / line.GetV0().norm());
      
      // here the particle is actually moved along the trajectory to new position:
      // std::visit(setup::ParticleUpdate<Particle>{vParticle}, step);
      vParticle.SetMomentum(direction * vParticle.GetMomentum().norm());
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

        if (min_distance != distance_max && min_distance != magMaxLength) {
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
