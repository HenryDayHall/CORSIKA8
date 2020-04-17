/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <limits>

#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

namespace corsika {

   template <class TDerived>
   class LinearApproximationIntegrator
   {

     auto const& GetImplementation() const;

  public:

     inline auto IntegrateGrammage( corsika::Trajectory<corsika::Line> const& line,
        corsika::units::si::LengthType length) const;

    inline auto ArclengthFromGrammage( corsika::Trajectory<corsika::Line> const& line,
        corsika::units::si::GrammageType grammage) const ;

    inline auto MaximumLength(corsika::Trajectory<corsika::Line> const& line,
                       [[maybe_unused]] double relError) const ;
  };

} // namespace corsika::environment

#include <corsika/detail/media/LinearApproximationIntegrator.inl>
