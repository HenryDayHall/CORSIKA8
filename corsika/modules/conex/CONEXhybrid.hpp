/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/SecondariesProcess.hpp>
#include <corsika/framework/process/CascadeEquationsProcess.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/media/ShowerAxis.hpp>

#include <corsika/modules/conex/CONEX_f.hpp>

namespace corsika {

  namespace conex {
    LengthType constexpr earthRadius{6371315 * meter};
  } // namespace conex

  class CONEXhybrid : public CascadeEquationsProcess<CONEXhybrid>,
                      public SecondariesProcess<CONEXhybrid> {

  public:
    CONEXhybrid(Point center, ShowerAxis const& showerAxis, LengthType groundDist,
                LengthType injectionHeight, HEPEnergyType primaryEnergy, PDGCode pdg);

    /**
     * Main entry point to pass new particle data towards CONEX. If a
     * particles is selected for CONEX, it is removed from the CORSIKA
     * 8 stack.
     */
    template <typename TStackView>
    void doSecondaries(TStackView&);

    /**
     * init currently needs to be called to initializa a new
     * event. All tables are cleared, etc.
     */
    void initCascadeEquations();

    /**
     * Cascade equations are solved basoned on the data in the tables
     */
    template <typename TStack>
    void doCascadeEquations(TStack& stack);

    /**
     * Internal function to fill particle data inside CONEX
     * tables. Only e.m. particles are selected right now.
     */
    bool addParticle(Code pid, HEPEnergyType energy, HEPEnergyType mass,
                     Point const& position, Vector<dimensionless_d> const& direction,
                     TimeType t);

    CoordinateSystemPtr const& getObserverCS() const { return conexObservationCS_; }

    HEPEnergyType getEnergyEM() const;
    void reset();

  private:
    // data members
    //! CONEX e.m. particle codes
    static std::array<std::pair<Code, int>, 3> constexpr egs_em_codes_{
        {{Code::Photon, 0}, {Code::Electron, -1}, {Code::Positron, -1}}};

    Point const center_; //!< center of CONEX Earth
    ShowerAxis const& showerAxis_;
    LengthType groundDist_;       //!< length from injection point to shower core
    LengthType injectionHeight_;  //!< starting height of primary particle
    HEPEnergyType primaryEnergy_; //!< primary particle energy
    PDGCode primaryPDG_;          //!< primary particle PDG
    Point const showerCore_;      //!< shower core
    CoordinateSystemPtr const conexObservationCS_; //!< CONEX observation frame
    DirectionVector const x_sf_,
        y_sf_; //!< unit vectors of CONEX shower frame, z_sf is shower axis direction
    HEPEnergyType energy_em_;
  };
} // namespace corsika

#include <corsika/detail/modules/conex/CONEXhybrid.inl>
