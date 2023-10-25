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
#include <corsika/modules/writers/ParticleWriterParquet.hpp>
#include <corsika/modules/writers/WriterOff.hpp>
#include <corsika/framework/core/Step.hpp>

namespace corsika {

  /**
   * @ingroup Modules
   * @{
   *
   * The ObservationPlane writes PDG codes, kinetic energies, locations and momentum unit
   * vectors of particles with respect to the central point of the plane into its output
   * file. By default, the particles are considered "absorbed" afterwards. You can also
   * set the ObservationPlane as non-absorbing.
   *
   * The default output format is parquet.
   *
   * **Note/Limitation:** as discussed in
   * https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/-/issues/397
   * you cannot put two ObservationPlanes exactly on top of each
   * other. Even if one of them is "permeable". You have to put a
   * small gap in between the two plane in such a scenario, or develop
   * another more specialized output class.
   */
  template <typename TTracking, typename TOutput = ParticleWriterParquet>
  class ObservationPlane : public ContinuousProcess<ObservationPlane<TTracking, TOutput>>,
                           public TOutput {

    using TOutput::write;

  public:
    /**
     * Construct a new Observation Plane object.
     *
     * @tparam TArgs
     * @param plane The plane.
     * @param x_dir The x-direction/axis.
     * @param absorbing Flag to make the plane absorbing.
     * @param outputArgs
     */
    template <typename... TArgs>
    ObservationPlane(Plane const& plane, DirectionVector const& x_dir,
                     bool const absorbing = true, TArgs&&... outputArgs);

    ~ObservationPlane() {}

    template <typename TParticle>
    ProcessReturn doContinuous(Step<TParticle>&, bool const stepLimit);

    template <typename TParticle, typename TTrajectory>
    LengthType getMaxStepLength(TParticle const&, TTrajectory const& vTrajectory);

    YAML::Node getConfig() const;

  private:
    Plane const plane_;
    DirectionVector const xAxis_;
    DirectionVector const yAxis_;
    bool const deleteOnHit_;
  };
  //! @}
} // namespace corsika

#include <corsika/detail/modules/ObservationPlane.inl>
