/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <cassert>
#include <cmath>
#include <limits>

//FIXME: importing what from BOOST ?
#include <boost/type_index.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/random/ExponentialDistribution.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>
#include <corsika/framework/sequence/ProcessReturn.hpp>
#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

//FIXME: importing what from BOOST ?
//using boost::typeindex::type_id_with_cvr;

#include <fstream>

/**
 * The cascade namespace assembles all objects needed to simulate full particles cascades.
 */

namespace corsika {

  /**
   * \class Cascade
   *
   * The Cascade class is constructed from template arguments making
   * it very versatile. Via the template arguments physics models are
   * plugged into the cascade simulation.
   *
   * <b>TTracking</b> must be a class according to the
   * TrackingInterface providing the functions:
   *
   * <code>
   * auto GetTrack(Particle const& p)</auto>,
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
  	  	  	  typename TStackView = corsika::StackView>
  class Cascade
  {

    typedef typename TStack::ParticleType Particle;
    typedef std::remove_pointer_t<decltype(((Particle*)nullptr)->GetNode())> VolumeTreeNode;
    typedef typename VolumeTreeNode::IModelProperties MediumInterface;

  public:

    Cascade() = delete;

    Cascade(corsika::Environment<MediumInterface> const& env, TTracking& tr,
    		 TProcessList& pl, TStack& stack):
    	fEnvironment(env),
		fTracking(tr),
		fProcessSequence(pl),
		fStack(stack)
    { }

    /**
     * The Init function is called before the actual cascade simulations.
     * All components of the Cascade simulation must be configured here.
     */
    void Init();

    /**
     * set the nodes for all particles on the stack according to their numerical
     * position
     */
    void SetNodes();

    /**
     * The Run function is the main simulation loop, which processes
     * particles from the Stack until the Stack is empty.
     */
    void Run();

    /**
     * Force an interaction of the top particle of the stack at its current position.
     * Note that SetNodes() or an equivalent procedure needs to be called first if you
     * want to call forceInteraction() for the primary interaction.
     */
    void forceInteraction();

  private:

    void Step(Particle& vParticle);

    auto decay(Particle& particle, decltype(std::declval<TStackView>().GetProjectile()) projectile);

    auto interaction(particle_type& particle, decltype(std::declval<TStackView>().GetProjectile()) projectile) ;

    corsika::Environment<MediumInterface> const& fEnvironment;
    TTracking& fTracking;
    TProcessList& fProcessSequence;
    TStack& fStack;
    corsika::RNG& fRNG =
        corsika::RNGManager::GetInstance().GetRandomStream("cascade");

  };

} // namespace corsika

#include <corsika/detail/framework/core/Cascade.inl>
