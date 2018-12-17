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

    void setHadronsUnstable() {
      // name? also makes EM particles stable

      // loop over all particles in sibyll
      // should be changed to loop over human readable list
      // i.e. corsika::particles::ListOfParticles()
      std::cout << "Sibyll: setting hadrons unstable.." << std::endl;
      // make ALL particles unstable, then set EM stable
      for (auto& p : corsika2sibyll) {
        // std::cout << (int)p << std::endl;
        const int sibCode = (int)p;
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
      setup::Stack ds;
      ds.NewParticle().SetPID(corsika::particles::Code::Proton);
      ds.NewParticle().SetPID(corsika::particles::Code::Neutron);
      ds.NewParticle().SetPID(corsika::particles::Code::Electron);
      ds.NewParticle().SetPID(corsika::particles::Code::Positron);
      ds.NewParticle().SetPID(corsika::particles::Code::NuE);
      ds.NewParticle().SetPID(corsika::particles::Code::NuEBar);
      ds.NewParticle().SetPID(corsika::particles::Code::MuMinus);
      ds.NewParticle().SetPID(corsika::particles::Code::MuPlus);
      ds.NewParticle().SetPID(corsika::particles::Code::NuMu);
      ds.NewParticle().SetPID(corsika::particles::Code::NuMuBar);

      for (auto& p : ds) {
        int s_id = process::sibyll::ConvertToSibyllRaw(p.GetPID());
        // set particle stable by setting table value negative
        //	cout << "Sibyll: setting " << p.GetPID() << "(" << s_id << ")"
        //     << " stable in Sibyll .." << endl;
        s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
        p.Delete();
      }
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
      setup::Stack ds;
      ds.NewParticle().SetPID(particles::Code::PiPlus);
      ds.NewParticle().SetPID(particles::Code::PiMinus);
      ds.NewParticle().SetPID(particles::Code::KPlus);
      ds.NewParticle().SetPID(particles::Code::KMinus);
      ds.NewParticle().SetPID(particles::Code::K0Long);
      ds.NewParticle().SetPID(particles::Code::K0Short);

      for (auto& p : ds) {
        int s_id = process::sibyll::ConvertToSibyllRaw(p.GetPID());
        // set particle stable by setting table value negative
        s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
        p.Delete();
      }
    }

    class Decay : public corsika::process::DecayProcess<Decay> {
    public:
      Decay() {}
      void Init() {
        setHadronsUnstable();
        setTrackedParticlesStable();
      }

      void setAllStable() {
        // name? also makes EM particles stable

        // loop over all particles in sibyll
        // should be changed to loop over human readable list
        // i.e. corsika::particles::ListOfParticles()
        for (auto& p : corsika2sibyll) {
          // std::cout << (int)p << std::endl;
          const int sibCode = (int)p;
          // skip unknown and antiparticles
          if (sibCode < 1) continue;
          std::cout << "Sibyll: Decay: setting "
                    << ConvertFromSibyll(static_cast<SibyllCode>(sibCode)) << " stable"
                    << std::endl;
          s_csydec_.idb[sibCode - 1] = -1 * abs(s_csydec_.idb[sibCode - 1]);
          std::cout << "decay table value: " << s_csydec_.idb[sibCode - 1] << std::endl;
        }
      }

      friend void setHadronsUnstable();

      template <typename Particle>
      double GetLifetime(Particle& p) const {
        corsika::units::hep::EnergyType E = p.GetEnergy();
        corsika::units::hep::MassType m = corsika::particles::GetMass(p.GetPID());

        // const MassDensityType density = 1.25e-3 * kilogram / (1_cm * 1_cm * 1_cm);

        const double gamma = E / m;

        const TimeType t0 = particles::GetLifetime(p.GetPID());
        cout << "Decay: code: " << (p.GetPID()) << endl;
        cout << "Decay: MinStep: t0: " << t0 << endl;
        cout << "Decay: MinStep: gamma: " << gamma << endl;
        // cout << "Decay: MinStep: density: " << density << endl;
        // return as column density
        // const double x0 = density * t0 * gamma * constants::c / kilogram * 1_cm * 1_cm;
        // cout << "Decay: MinStep: x0: " << x0 << endl;
        const double lifetime = gamma * t0 / 1_s;
        cout << "Decay: MinStep: tau: " << lifetime << endl;
        // int a = 1;
        // const double x = -x0 * log(s_rndm_(a));
        // cout << "Decay: next decay: " << x << endl;
        return lifetime;
      }

      template <typename Particle, typename Stack>
      void DoDecay(Particle& p, Stack& s) const {
        SibStack ss;
        ss.Clear();
        // copy particle to sibyll stack
        auto pin = ss.NewParticle();
        pin.SetPID(process::sibyll::ConvertToSibyllRaw(p.GetPID()));
        pin.SetEnergy(p.GetEnergy());
        pin.SetMomentum(p.GetMomentum());
        // remove original particle from corsika stack
        p.Delete();
        // set all particles/hadrons unstable
        setHadronsUnstable();
        // call sibyll decay
        std::cout << "Decay: calling Sibyll decay routine.." << std::endl;
        decsib_();
        // print output
        int print_unit = 6;
        sib_list_(print_unit);
        // copy particles from sibyll stack to corsika
        int i = -1;
        for (auto& psib : ss) {
          ++i;
          // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
          if (abs(s_plist_.llist[i]) > 100) continue;
          // add to corsika stack
          // cout << "decay product: " << process::sibyll::ConvertFromSibyll(
          // psib.GetPID() ) << endl;
          auto pnew = s.NewParticle();
          pnew.SetEnergy(psib.GetEnergy());
          pnew.SetPID(process::sibyll::ConvertFromSibyll(psib.GetPID()));
          pnew.SetMomentum(psib.GetMomentum());
        }
        // empty sibyll stack
        ss.Clear();
      }
    };
  } // namespace sibyll
} // namespace corsika::process

#endif
