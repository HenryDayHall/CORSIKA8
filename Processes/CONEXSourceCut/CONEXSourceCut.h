/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/ShowerAxis.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/process/conex_source_cut/CONEX_f.h>

namespace conex {
  corsika::units::si::LengthType constexpr earthRadius{6371315 *
                                                       corsika::units::si::meter};
} // namespace conex

namespace corsika::process {
  namespace conex_source_cut {
    class CONEXSourceCut : public process::SecondariesProcess<CONEXSourceCut> {

    public:
      CONEXSourceCut(geometry::Point center, environment::ShowerAxis const& showerAxis,
                     units::si::LengthType groundDist,
                     units::si::LengthType injectionHeight,
                     units::si::HEPEnergyType primaryEnergy,
                     particles::PDGCode primaryID);
      corsika::process::EProcessReturn DoSecondaries(corsika::setup::StackView&);

      void SolveCE();

      void addParticle(int egs_pid, units::si::HEPEnergyType energy,
                       units::si::HEPEnergyType mass, geometry::Point const& position,
                       geometry::Vector<units::si::dimensionless_d> const& direction,
                       units::si::TimeType t);

      auto const& GetObserverCS() const { return conexObservationCS_; }

    private:
      //! CONEX e.m. particle codes
      static std::array<std::pair<particles::Code, int>, 3> constexpr egs_em_codes_{
          {{particles::Code::Gamma, 0},
           {particles::Code::Electron, -1},
           {particles::Code::Positron, -1}}};

      geometry::Point const center_; //!< center of CONEX Earth
      environment::ShowerAxis const& showerAxis_;
      units::si::LengthType groundDist_;                    //!< length from injection point to shower core
      geometry::Point const showerCore_;                    //!< shower core
      geometry::CoordinateSystem const conexObservationCS_; //!< CONEX observation frame
      geometry::Vector<units::si::dimensionless_d> const x_sf_,
          y_sf_; //!< unit vectors of CONEX shower frame, z_sf is shower axis direction
    };
  } // namespace conex_source_cut
} // namespace corsika::process
