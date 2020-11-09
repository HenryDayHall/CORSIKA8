#pragma once

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/logging/Logging.h>
#include <corsika/geometry/Intersections.hpp>

namespace corsika::process::tracking {

  /**
   * \class Intersect
   *
   *
   *
   **/

  template <typename TDerived>
  class Intersect {

  protected:
    template <typename TParticle>
    auto nextIntersect(const TParticle& particle) const {
      using namespace corsika::units::si;
      using namespace corsika::geometry;

      const Point& initialPosition = particle.GetPosition();

      typedef
          typename std::remove_reference<decltype(*particle.GetNode())>::type node_type;
      node_type& volumeNode = *particle.GetNode();
      C8LOG_DEBUG("volumeNode={}, numericallyInside={} ", fmt::ptr(&volumeNode),
                  volumeNode.GetVolume().Contains(initialPosition));

      auto const velocity =
          particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;

      // for the event of magnetic fields and curved trajectories, we need to limit
      // maximum step-length since we need to follow curved
      // trajectories segment-wise -- at least if we don't employ concepts as "Helix
      // Trajectories" or similar
      const auto& magneticfield =
          volumeNode.GetModelProperties().GetMagneticField(initialPosition);
      const auto magnitudeB = magneticfield.norm();
      const int chargeNumber = particle.GetChargeNumber();
      auto const momentumVerticalMag =
          particle.GetMomentum() -
          particle.GetMomentum().parallelProjectionOnto(magneticfield);
      LengthType const gyroradius =
          (chargeNumber == 0 || magnitudeB == 0_T
               ? std::numeric_limits<TimeType::value_type>::infinity() * 1_m
               : momentumVerticalMag.norm() * 1_V /
                     (corsika::units::constants::c * abs(chargeNumber) * magnitudeB *
                      1_eV));
      const double maxRadians = 0.01;
      const LengthType steplimit = 2 * cos(maxRadians) * sin(maxRadians) * gyroradius;
      C8LOG_DEBUG("gyroradius {}, steplimit: {} m = {} s", gyroradius, steplimit,
                  steplimit / velocity.norm());

      // start values:
      TimeType minTime = steplimit / velocity.norm();
      node_type* minNode = &volumeNode;

      // determine the first geometric collision with any other Volume boundary

      // first check, where we leave the current volume
      // this assumes our convention, that all Volume primitives must be convex
      // thus, the last entry is always the exit point
      const Intersections time_intersections_curr =
          TDerived::Intersect(particle, volumeNode);
      C8LOG_TRACE("curr node {}, parent node {} ", fmt::ptr(&volumeNode),
                  fmt::ptr(volumeNode.GetParent()));
      C8LOG_DEBUG("intersection times with currentLogicalVolumeNode: {} s and {} s",
                  time_intersections_curr.getEntry() / 1_s,
                  time_intersections_curr.getExit() / 1_s);
      if (time_intersections_curr.getExit() <= minTime) {
        minTime =
            time_intersections_curr.getExit(); // we exit currentLogicalVolumeNode here
        minNode = volumeNode.GetParent();
      }

      // where do we collide with any of the next-tree-level volumes
      // entirely contained by currentLogicalVolumeNode
      for (const auto& node : volumeNode.GetChildNodes()) {

        const Intersections time_intersections = TDerived::Intersect(particle, *node);
        C8LOG_DEBUG("intersection times with child volume {} : enter {} s, exit {} s",
                    fmt::ptr(node), time_intersections.getEntry() / 1_s,
                    time_intersections.getExit() / 1_s);

        const auto t_entry = time_intersections.getEntry();
        const auto t_exit = time_intersections.getExit();
        C8LOG_TRACE("children t-entry: {}, t-exit: {}, smaller? {} ", t_entry, t_exit,
                    t_entry <= minTime);
        // note, theoretically t can even be smaller than 0 since we
        // KNOW we can't yet be in this volume yet, so we HAVE TO
        // enter it IF exit point is not also in the "past"!
        if (t_exit > 0_s && t_entry <= minTime) { // enter volumen child here
          minTime = t_entry;
          minNode = node.get();
        }
      }

      // these are volumes from the previous tree-level that are cut-out partly from the
      // current volume
      for (node_type* node : volumeNode.GetExcludedNodes()) {

        const Intersections time_intersections = TDerived::Intersect(particle, *node);
        C8LOG_DEBUG("intersection times with exclusion volume {} : enter {} s, exit {} s",
                    fmt::ptr(node), time_intersections.getEntry() / 1_s,
                    time_intersections.getExit() / 1_s);
        const auto t_entry = time_intersections.getEntry();
        const auto t_exit = time_intersections.getExit();
        C8LOG_TRACE("children t-entry: {}, t-exit: {}, smaller? {} ", t_entry, t_exit,
                    t_entry <= minTime);
        // note, theoretically t can even be smaller than 0 since we
        // KNOW we can't yet be in this volume yet, so we HAVE TO
        // enter it IF exit point is not also in the "past"!
        if (t_exit > 0_s && t_entry <= minTime) { // enter volumen child here
          minTime = t_entry;
          minNode = node;
        }
      }
      C8LOG_TRACE("t-intersect: {}, node {} ", minTime, fmt::ptr(minNode));
      return std::make_tuple(minTime, minNode);
    }
  }; // namespace corsika::process::tracking
} // namespace corsika::process::tracking
