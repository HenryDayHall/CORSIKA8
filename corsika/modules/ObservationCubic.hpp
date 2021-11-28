#pragma once

#include <corsika/framework/geometry/Cubic.hpp>

#include <corsika/modules/writers/ObservationCubicWriterParquet.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>


namespace corsika {

  /**
     @ingroup Modules
     @{

     The ObservationCubic writes PDG codes, energies, and distances of particles to the
     central point of the plane into its output file. The particles are considered
     "absorbed" afterwards.

     **Note/Limitation:** as discussed in
     https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/issues/397
     you cannot put two ObservationCubics exactly on top of each
     other. Even if one of them is "permeable". You have to put a
     small gap in between the two plane in such a scenario, or develop
     another more specialized output class.
   */
  template <typename TTracking, typename TOutputWriter = ObservationCubicWriterParquet>
  class ObservationCubic
      : public Cubic,
        public ContinuousProcess<ObservationCubic<TTracking, TOutputWriter>>,
        public TOutputWriter {

  public:
    ObservationCubic(Point const& center, CoordinateSystemPtr cs,
    LengthType const x, LengthType const y, LengthType const z, bool = true);

    template <typename TParticle, typename TTrajectory>
    ProcessReturn doContinuous(TParticle& vParticle, TTrajectory& vTrajectory,
                               bool const stepLimit);

    template <typename TParticle, typename TTrajectory>
    LengthType getMaxStepLength(TParticle const&, TTrajectory const& vTrajectory);

    void showResults() const;
    void reset();
    HEPEnergyType getEnergy() const { return energy_; }
    YAML::Node getConfig() const;

  private:
    bool const deleteOnHit_;
    HEPEnergyType energy_;
    unsigned int count_;

  };
  //! @}
} // namespace corsika



#include <corsika/detail/modules/ObservationCubic.inl>