/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/ProcessReturn.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/ExponentialDistribution.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>
#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/framework/logging/Logging.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <cassert>
#include <cmath>
#include <limits>
#include <type_traits>

/**
 * The cascade namespace assembles all objects needed to simulate full particles cascades.
 */

namespace corsika {

  /**
   *
   * The Cascade class is constructed from template arguments making
   * it very versatile. Via the template arguments physics models are
   * plugged into the cascade simulation.
   *
   * <b>TTracking</b> must be a class according to the
   * TrackingInterface providing the functions:
   *
   * <code>
   * auto getTrack(Particle const& p)</auto>,
   * with the return type <code>geometry::Trajectory<corsika::Line>
   * </code>
   *
   * <b>TProcessList</b> must be a ProcessSequence.   *
   * <b>Stack</b> is the storage object for particle data, i.e. with
   * Particle class type <code>Stack::ParticleType</code>
   *
   *
   */
  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView = corsika::setup::StackView>
  class Cascade {

    typedef typename TStack::particle_type Particle;
    typedef std::remove_pointer_t<decltype(((Particle*)nullptr)->getNode())>
        VolumeTreeNode;
    typedef typename VolumeTreeNode::IModelProperties MediumInterface;

  public:
    Cascade() = delete;

    Cascade(corsika::Environment<MediumInterface> const& env, TTracking& tr,
            TProcessList& pl, TStack& stack)
        : environment_(env)
        , tracking_(tr)
        , sequence_(pl)
        , stack_(stack) {
      CORSIKA_LOG_INFO(c8_ascii_);
      if constexpr (TStackView::has_event) {
        CORSIKA_LOG_INFO(" - With full cascade HISTORY.");
      }
    }

    /**
     * The Init function is called before the actual cascade simulations.
     * All components of the Cascade simulation must be configured here.
     */
    void init();

    /**
     * set the nodes for all particles on the stack according to their numerical
     * position
     */
    void setNodes();

    /**
     * The Run function is the main simulation loop, which processes
     * particles from the Stack until the Stack is empty.
     */
    void run();

    /**
     * Force an interaction of the top particle of the stack at its current position.
     * Note that setNodes() or an equivalent procedure needs to be called first if you
     * want to call forceInteraction() for the primary interaction.
     */
    void forceInteraction();

  private:
    void step(Particle& vParticle);

    ProcessReturn decay(TStackView& view);
    ProcessReturn interaction(TStackView& view);
    void setEventType(TStackView& view, history::EventType);

    // data members
    corsika::Environment<MediumInterface> const& environment_;
    TTracking& tracking_;
    TProcessList& sequence_;
    TStack& stack_;
    corsika::default_prng_type& rng_ =
        corsika::RNGManager::getInstance().getRandomStream("cascade");
    unsigned int count_ = 0;

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

} // namespace corsika

#include <corsika/detail/framework/core/Cascade.inl>
