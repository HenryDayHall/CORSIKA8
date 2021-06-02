/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/modules/writers/ObservationPlaneWriterParquet.hpp>

namespace corsika {

  /**
     @ingroup Modules
     @{

     The ObservationPlane writes PDG codes, energies, and distances of particles to the
     central point of the plane into its output file. The particles are considered
     "absorbed" afterwards.

     **Note/Limitation:** as discussed in
     https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/issues/397
     you cannot put two ObservationPlanes exactly on top of each
     other. Even if one of them is "permeable". You have to put a
     small gap in between the two plane in such a scenario, or develop
     another more specialized output class.
   */
  template <typename TTracking, typename TOutputWriter = ObservationPlaneWriterParquet>
  class ObservationPlane
      : public ContinuousProcess<ObservationPlane<TTracking, TOutputWriter>>,
        public TOutputWriter {

  public:
    template <typename... TArgs>
    ObservationPlane(Plane const&, DirectionVector const&, bool const = true,
                     TArgs&&... args);

    ~ObservationPlane() {}

    template <typename TParticle, typename TTrajectory>
    ProcessReturn doContinuous(TParticle& vParticle, TTrajectory& vTrajectory,
                               bool const stepLimit);

    template <typename TParticle, typename TTrajectory>
    LengthType getMaxStepLength(TParticle const&, TTrajectory const& vTrajectory);

    void showResults() const;
    void reset();
    HEPEnergyType getEnergyGround() const { return energy_ground_; }
    YAML::Node getConfig() const;

  private:
    Plane const plane_;
    bool const deleteOnHit_;
    HEPEnergyType energy_ground_;
    unsigned int count_ground_;
    DirectionVector const xAxis_;
    DirectionVector const yAxis_;
  };
  //! @}
} // namespace corsika

#include <corsika/detail/modules/ObservationPlane.inl>
