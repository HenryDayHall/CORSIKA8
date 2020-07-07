/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_particle_cut_CONEXSourceCut_h_
#define _corsika_process_particle_cut_CONEXSourceCut_h_

#include <ConexDynamicInterface.h>

#include <corsika/environment/ShowerAxis.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace conex {
  extern "C" {
    // ipart,energy,theta,phi,dimpact,ioseed
    void conexrun_(int& ipart, double& energy, double& theta, double& phi, double& dimpact,
                   int ioseed[3]);
    void conexcascade_();
    void hadroncascade_(int&, int&, int&, int&);
    void solvemomentequations_(int&);
    void show_(int& iqi, double& ei, double& xmi, double& ymi, double& zmi, double& dmi,
               double& xi, double& yi, double& zi, double& tmi, double& ui, double& vi,
               double& wi, int& iri, double& wti, int& latchi);

    int get_number_of_depth_bins_(); 

    void get_shower_data_(const int&, const int&, const int&, float&, float&,
			float&, float&, float&);
    void get_shower_edep_(const int&, const int&, float&, float&);
    void get_shower_muon_(const int&, const int&, float&, float&);
    void get_shower_gamma_(const int&, const int&, float&);
    void get_shower_electron_(const int&, const int&, float&);
    void get_shower_hadron_(const int&, const int&, float&);
  }
  corsika::units::si::LengthType constexpr earthRadius{6371315 *
                                                       corsika::units::si::meter};
} // namespace conex

namespace corsika::process {
  namespace conex_source_cut {
    class CONEXSourceCut : public process::SecondariesProcess<CONEXSourceCut> {

    public:
      CONEXSourceCut(geometry::Point center, environment::ShowerAxis showerAxis,
                     units::si::LengthType groundDist, /*units::si::GrammageType Xcut,*/
                     units::si::HEPEnergyType primaryEnergy,
                     particles::PDGCode primaryID);
      corsika::process::EProcessReturn DoSecondaries(corsika::setup::StackView&);

      void Init();

      void SolveCE();

    private:
      //! CONEX e.m. particle codes
      static std::array<std::pair<particles::Code, int>, 3> constexpr egs_em_codes_{
          {{particles::Code::Gamma, 0},
           {particles::Code::Electron, -1},
           {particles::Code::Positron, -1}}};

      geometry::Point const center_; //!< center of CONEX Earth
      environment::ShowerAxis const& showerAxis_;
      units::si::LengthType groundDist_; //!< length from injection point to shower core
      geometry::CoordinateSystem const conexObservationCS_; //!< CONEX observation frame
      geometry::Vector<units::si::dimensionless_d> const x_sf_,
          y_sf_; //!< unit vectors of CONEX shower frame, z_sf is shower axis direction
    };
  } // namespace conex_source_cut
} // namespace corsika::process

#endif
