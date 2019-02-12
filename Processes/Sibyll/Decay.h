
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_process_sibyll_decay_h_
#define _include_corsika_process_sibyll_decay_h_

#include <corsika/process/DecayProcess.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/particles/ParticleProperties.h>

namespace corsika::process {

  namespace sibyll {

    class Decay : public corsika::process::DecayProcess<Decay> {
      int fCount = 0;

    public:
      Decay() {}
      ~Decay() { std::cout << "Sibyll::Decay n=" << fCount << std::endl; }
      void Init() {
        setHadronsUnstable();
        setTrackedParticlesStable();
      }

      void setTrackedParticlesStable() {
        /*
           Sibyll is hadronic generator
           only hadrons decay
         */
        // set particles unstable
        setHadronsUnstable();
        // make tracked particles stable
        std::cout << "Interaction: setting tracked hadrons stable.." << std::endl;
        const std::vector<corsika::particles::Code> particleList = {
            particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
            particles::Code::KMinus, particles::Code::K0Long,  particles::Code::K0Short};

        for (auto p : particleList) {
          // set particle stable by setting table value negative
          const int sibid = process::sibyll::ConvertToSibyllRaw(p);
          s_csydec_.idb[sibid - 1] = (-1) * abs(s_csydec_.idb[sibid - 1]);
        }
      }

      void setUnstable(const corsika::particles::Code pCode) {
        int s_id = process::sibyll::ConvertToSibyllRaw(pCode);
        s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
      }

      void setStable(const corsika::particles::Code pCode) {
        int s_id = process::sibyll::ConvertToSibyllRaw(pCode);
        s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
      }

      void setAllStable() {
        // name? also makes EM particles stable

        using std::cout;
        using std::endl;

        std::cout << "Decay: setting all particles stable.." << std::endl;

        // loop over all particles in sibyll
        // should be changed to loop over human readable list
        // i.e. corsika::particles::ListOfParticles()
        for (auto& p : corsika2sibyll) {
          // std::cout << (int)p << std::endl;
          const int sibCode = static_cast<int>(p);
          // skip unknown and antiparticles
          if (sibCode < 1) continue;
          s_csydec_.idb[sibCode - 1] = -1 * abs(s_csydec_.idb[sibCode - 1]);
        }
      }

      void setHadronsUnstable() {

        using std::cout;
        using std::endl;
        using namespace corsika::units::si;

        // name? also makes EM particles stable

        // loop over all particles in sibyll
        // should be changed to loop over human readable list
        // i.e. corsika::particles::ListOfParticles()
        std::cout << "Sibyll: setting hadrons unstable.." << std::endl;
        // make ALL particles unstable, then set EM stable
        for (int sibCode : corsika2sibyll) {
          // std::cout << (int)p << std::endl;
          // skip unknown and antiparticles
          if (sibCode < 1) continue;
          // std::cout << "Sibyll: Decay: setting " << ConvertFromSibyll(
          // static_cast<SibyllCode> ( sibCode ) ) << " unstable" << std::endl;
          s_csydec_.idb[sibCode - 1] = abs(s_csydec_.idb[sibCode - 1]);
          // std::cout << "decay table value: " << s_csydec_.idb[ sibCode - 1 ] <<
          // std::endl;
        }
        // set Leptons and Proton and Neutron stable
        // use stack to loop over particles
        constexpr corsika::particles::Code particleList[] = {
            corsika::particles::Code::Proton,   corsika::particles::Code::Neutron,
            corsika::particles::Code::Electron, corsika::particles::Code::Positron,
            corsika::particles::Code::NuE,      corsika::particles::Code::NuEBar,
            corsika::particles::Code::MuMinus,  corsika::particles::Code::MuPlus,
            corsika::particles::Code::NuMu,     corsika::particles::Code::NuMuBar};

        for (auto p : particleList) {
          // set particle stable by setting table value negative
          //	cout << "Sibyll: setting " << p.GetPID() << "(" << s_id << ")"
          //     << " stable in Sibyll .." << endl;
          const int sibid = process::sibyll::ConvertToSibyllRaw(p);
          s_csydec_.idb[sibid - 1] = (-1) * abs(s_csydec_.idb[sibid - 1]);
        }
      }

      template <typename Particle>
      corsika::units::si::TimeType GetLifetime(Particle const& p) {
        using std::cout;
        using std::endl;
        using namespace corsika::units::si;

        corsika::units::si::HEPEnergyType E = p.GetEnergy();
        corsika::units::si::HEPMassType m = corsika::particles::GetMass(p.GetPID());

        const double gamma = E / m;

        const corsika::units::si::TimeType t0 =
            corsika::particles::GetLifetime(p.GetPID());
        auto const lifetime = gamma * t0;

        const auto mkin =
            (E * E - p.GetMomentum().squaredNorm()); // delta_mass(p.GetMomentum(), E, m);
        cout << "Decay: code: " << p.GetPID() << endl;
        cout << "Decay: MinStep: t0: " << t0 << endl;
        cout << "Decay: MinStep: energy: " << E / 1_GeV << " GeV" << endl;
        cout << "Decay: momentum: " << p.GetMomentum().GetComponents() / 1_GeV << " GeV"
             << endl;
        cout << "Decay: momentum: shell mass-kin. inv. mass " << mkin / 1_GeV / 1_GeV
             << " " << m / 1_GeV * m / 1_GeV << endl;
        // cout << "Decay: sib mass: " << s_mass1_.am2[
        // process::sibyll::ConvertToSibyllRaw(p.GetPID()) ] << endl;
        auto sib_id = process::sibyll::ConvertToSibyllRaw(p.GetPID());
        cout << "Decay: sib mass: " << get_sibyll_mass2(sib_id) << endl;
        cout << "Decay: MinStep: gamma: " << gamma << endl;
        cout << "Decay: MinStep: tau: " << lifetime << endl;

        return lifetime;
      }

      template <typename Particle, typename Stack>
      void DoDecay(Particle& p, Stack&) {
        using corsika::geometry::Point;
        using namespace corsika::units::si;

        fCount++;
        SibStack ss;
        ss.Clear();
        const corsika::particles::Code pCode = p.GetPID();
        // copy particle to sibyll stack
        ss.AddParticle(process::sibyll::ConvertToSibyllRaw(pCode), p.GetEnergy(),
                       p.GetMomentum(),
                       // setting particle mass with Corsika values, may be inconsistent
                       // with sibyll internal values
                       // TODO: #warning setting particle mass with Corsika values, may be
                       // inconsistent with sibyll internal values
                       corsika::particles::GetMass(pCode));
        // remember position
        Point const decayPoint = p.GetPosition();
        TimeType const t0 = p.GetTime();
        // remove original particle from corsika stack
        p.Delete();
        // set all particles/hadrons unstable
        // setHadronsUnstable();
        setUnstable(pCode);
        // call sibyll decay
        std::cout << "Decay: calling Sibyll decay routine.." << std::endl;
        decsib_();
        // reset to stable
        setStable(pCode);
        // print output
        int print_unit = 6;
        sib_list_(print_unit);

        // copy particles from sibyll stack to corsika
        for (auto& psib : ss) {
          // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
          if (psib.HasDecayed()) continue;
          // add to corsika stack
          p.AddSecondary(process::sibyll::ConvertFromSibyll(psib.GetPID()),
                         psib.GetEnergy(), psib.GetMomentum(), decayPoint, t0);
        }
        // empty sibyll stack
        ss.Clear();
      }
    };
  } // namespace sibyll
} // namespace corsika::process

#endif
