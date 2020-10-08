/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/observation_plane/ObservationPlane.h>
#include <corsika/logging/Logging.h>

#include <fstream>

using namespace corsika::process::observation_plane;
using namespace corsika::units::si;

ObservationPlane::ObservationPlane(
    geometry::Plane const& obsPlane,
    geometry::Vector<units::si::dimensionless_d> const& x_axis,
    std::string const& filename, bool deleteOnHit)
    : plane_(obsPlane)
    , outputStream_(filename)
    , deleteOnHit_(deleteOnHit)
    , energy_ground_(0_GeV)
    , count_ground_(0)
    , xAxis_(x_axis.normalized())
    , yAxis_(obsPlane.GetNormal().cross(xAxis_)) {
  outputStream_ << "#PDG code, energy / eV, x distance / m, y distance / m" << std::endl;
}

corsika::process::EProcessReturn ObservationPlane::DoContinuous(
    setup::Stack::ParticleType& particle, setup::Trajectory const& trajectory) {
  TimeType const timeOfIntersection =
      (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
      trajectory.GetV0().dot(plane_.GetNormal());

  if (timeOfIntersection < TimeType::zero()) { return process::EProcessReturn::eOk; }

  if (plane_.IsAbove(trajectory.GetR0()) == plane_.IsAbove(trajectory.GetPosition(1))) {
    return process::EProcessReturn::eOk;
  }

  const auto energy = particle.GetEnergy();
  auto const displacement = trajectory.GetPosition(1) - plane_.GetCenter();

  outputStream_ << static_cast<int>(particles::GetPDG(particle.GetPID())) << ' '
                << energy / 1_eV << ' '
                << displacement.dot(xAxis_) / 1_m << ' ' << displacement.dot(yAxis_) / 1_m
                << (trajectory.GetPosition(1) - plane_.GetCenter()).norm() / 1_m
                << std::endl;

  if (deleteOnHit_) {
    count_ground_++;
    energy_ground_ += energy;
    particle.Delete();
    return process::EProcessReturn::eParticleAbsorbed;
  } else {
    return process::EProcessReturn::eOk;
  }
}

LengthType ObservationPlane::MaxStepLength(setup::Stack::ParticleType const& vParticle,
                                           setup::Trajectory const& trajectory) {
  int chargeNumber;
  if (corsika::particles::IsNucleus(vParticle.GetPID())) {
    chargeNumber = vParticle.GetNuclearZ();
  } else {
    chargeNumber = corsika::particles::GetChargeNumber(vParticle.GetPID());
  }
  auto const* currentLogicalVolumeNode = vParticle.GetNode();
  auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(vParticle.GetPosition());
  geometry::Vector<SpeedType::dimension_type> const velocity = trajectory.GetV0();
  
  if (chargeNumber != 0 && plane_.GetNormal().dot(velocity.cross(magneticfield)) * 1_s / 1_m / 1_T > 0.0001) {
    auto const* currentLogicalVolumeNode = vParticle.GetNode();
    auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(vParticle.GetPosition());
    auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / 
            (velocity.GetSquaredNorm() * vParticle.GetEnergy() * 1_V); 
    LengthType MaxStepLength1 = 
      ( sqrt(velocity.dot(plane_.GetNormal()) * velocity.dot(plane_.GetNormal()) / velocity.GetSquaredNorm() - 
      (plane_.GetNormal().dot(trajectory.GetR0() - plane_.GetCenter()) * 
      plane_.GetNormal().dot(velocity.cross(magneticfield)) * 2 * k)) - 
      velocity.dot(plane_.GetNormal()) / velocity.GetNorm() ) / 
      (plane_.GetNormal().dot(velocity.cross(magneticfield)) * k);
    LengthType MaxStepLength2 = 
      ( - sqrt(velocity.dot(plane_.GetNormal()) * velocity.dot(plane_.GetNormal()) / velocity.GetSquaredNorm() - 
      (plane_.GetNormal().dot(trajectory.GetR0() - plane_.GetCenter()) * 
      plane_.GetNormal().dot(velocity.cross(magneticfield)) * 2 * k)) - 
      velocity.dot(plane_.GetNormal()) / velocity.GetNorm() ) / 
      (plane_.GetNormal().dot(velocity.cross(magneticfield)) * k);
      
    if (MaxStepLength1 <= 0_m && MaxStepLength2 <= 0_m) {
      return std::numeric_limits<double>::infinity() * 1_m;
    } else if (MaxStepLength1 <= 0_m || MaxStepLength2 < MaxStepLength1) {
	  std::cout << " distance to obs plane : " << MaxStepLength2 << std::endl;
      return MaxStepLength2 * 1.0001;
    } else if (MaxStepLength2 <= 0_m || MaxStepLength1 < MaxStepLength2) {
	  std::cout << " distance to obs plane : " << MaxStepLength1 << std::endl;
      return MaxStepLength1 * 1.0001;
    }
  } 
  TimeType const timeOfIntersection =
    (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
    trajectory.GetV0().dot(plane_.GetNormal());

  if (timeOfIntersection < TimeType::zero()) {
    return std::numeric_limits<double>::infinity() * 1_m;
  }

  auto const pointOfIntersection = trajectory.GetPosition(timeOfIntersection);
  return (trajectory.GetR0() - pointOfIntersection).norm() * 1.0001;
}

void ObservationPlane::ShowResults() const {
  C8LOG_INFO(
      " ******************************\n"
      " ObservationPlane: \n"
      " energy in ground (GeV)     :  {}\n"
      " no. of particles in ground :  {}\n"
      " ******************************",
      energy_ground_ / 1_GeV, count_ground_);
}

void ObservationPlane::Reset() {
  energy_ground_ = 0_GeV;
  count_ground_ = 0;
}
