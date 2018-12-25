#ifndef _corsika_process_sibyll_interaction_h_
#define _corsika_process_sibyll_interaction_h_

#include <corsika/process/InteractionProcess.h>

#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>
#include <corsika/process/sibyll/sibyll2.3c.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process::sibyll {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    mutable int fCount = 0;

  public:
    Interaction() {}
    ~Interaction() { std::cout << "Sibyll::Interaction n=" << fCount << std::endl; }

    void Init() {

      using corsika::random::RNGManager;
      using std::cout;
      using std::endl;

      corsika::random::RNGManager& rmng = corsika::random::RNGManager::GetInstance();
      rmng.RegisterRandomStream("s_rndm");

      // initialize Sibyll
      sibyll_ini_();
    }

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle& p, Track&) const {

      using namespace corsika::units;
      using namespace corsika::units::hep;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      // coordinate system, get global frame of reference
      CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      const particles::Code corsikaBeamId = p.GetPID();

      // beam particles for sibyll : 1, 2, 3 for p, pi, k
      // read from cross section code table
      const int kBeam = process::sibyll::GetSibyllXSCode(corsikaBeamId);
      const bool kInteraction = process::sibyll::CanInteract(corsikaBeamId);

      /*
         the target should be defined by the Environment,
         ideally as full particle object so that the four momenta
         and the boosts can be defined..
       */
      // target nuclei: A < 18
      // FOR NOW: assume target is oxygen
      const int kTarget = corsika::particles::Oxygen::GetNucleusA();

      const hep::MassType nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                                corsika::particles::Neutron::GetMass());
      hep::EnergyType Etot = p.GetEnergy() + nucleon_mass;
      MomentumVector Ptot(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      // FOR NOW: assume target is at rest
      MomentumVector pTarget(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      Ptot += p.GetMomentum();
      Ptot += pTarget;
      // calculate cm. energy
      const hep::EnergyType sqs = sqrt(Etot * Etot - Ptot.squaredNorm());
      const double Ecm = sqs / 1_GeV;

      std::cout << "Interaction: LambdaInt: \n"
                << " input energy: " << p.GetEnergy() / 1_GeV << endl
                << " beam can interact:" << kBeam << endl
                << " beam XS code:" << kBeam << endl
                << " beam pid:" << p.GetPID() << endl
                << " target mass number:" << kTarget << std::endl;

      if (kInteraction) {

        double prodCrossSection, dummy, dum1, dum2, dum3, dum4;
        double dumdif[3];

        if (kTarget == 1)
          sib_sigma_hp_(kBeam, Ecm, dum1, dum2, prodCrossSection, dumdif, dum3, dum4);
        else
          sib_sigma_hnuc_(kBeam, kTarget, Ecm, prodCrossSection, dummy);

        std::cout << "Interaction: "
                  << "MinStep: sibyll return: " << prodCrossSection << std::endl;
        si::CrossSectionType sig = prodCrossSection * 1_mbarn;
        std::cout << "Interaction: "
                  << "MinStep: CrossSection (mb): " << sig / 1_mbarn << std::endl;

        const si::MassType nucleon_mass =
            0.93827_GeV / corsika::units::constants::cSquared;
        std::cout << "Interaction: "
                  << "nucleon mass " << nucleon_mass << std::endl;
        // calculate interaction length in medium
        GrammageType int_length = kTarget * nucleon_mass / sig;
        std::cout << "Interaction: "
                  << "interaction length (g/cm2): " << int_length << std::endl;

        return int_length;
      }

      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }

    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle& p, Stack& s) const {

      using namespace corsika::units;
      using namespace corsika::units::hep;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      cout << "ProcessSibyll: "
           << "DoInteraction: " << p.GetPID() << " interaction? "
           << process::sibyll::CanInteract(p.GetPID()) << endl;
      if (process::sibyll::CanInteract(p.GetPID())) {
        const CoordinateSystem& rootCS =
            RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

        Point pOrig = p.GetPosition();
        TimeType tOrig = p.GetTime();
        // sibyll CS has z along particle momentum
        // FOR NOW: hard coded z-axis for corsika frame
        QuantityVector<length_d> const zAxis{0_m, 0_m, 1_m};
        QuantityVector<length_d> const xAxis{1_m, 0_m, 0_m};
        auto pt = [](MomentumVector p) {
          return sqrt(p.GetComponents()[0] * p.GetComponents()[0] +
                      p.GetComponents()[1] * p.GetComponents()[1]);
        };
        double theta = acos(p.GetMomentum().GetComponents()[2] / p.GetMomentum().norm());
        cout << "ProcessSibyll: polar angle between sibyllCS and rootCS: " << theta
             << endl;
        CoordinateSystem sibyllCS = rootCS.rotate(xAxis, theta);

        /*
           the target should be defined by the Environment,
           ideally as full particle object so that the four momenta
           and the boosts can be defined..

           here we need: GetTargetMassNumber() or GetTargetPID()??
                         GetTargetMomentum() (zero in EAS)
        */
        // FOR NOW: set target to oxygen
        const int kTarget = corsika::particles::Oxygen::
            GetNucleusA(); // env.GetTargetParticle().GetPID();

        // FOR NOW: target is always at rest
        const hep::MassType nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                                  corsika::particles::Neutron::GetMass());
        const EnergyType Etarget = 0_GeV + nucleon_mass;
        const auto pTarget = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
        cout << "target momentum (GeV/c): " << pTarget.GetComponents() / 1_GeV << endl;
        cout << "beam momentum (GeV/c): " << p.GetMomentum().GetComponents() / 1_GeV
             << endl;
        cout << "beam momentum in sibyll frame (GeV/c): "
             << p.GetMomentum().GetComponents(sibyllCS) / 1_GeV << endl;

        cout << "position of interaction: " << pOrig.GetCoordinates() << endl;
        cout << "time: " << tOrig << endl;

        // get energy of particle from stack
        /*
          stack is in GeV in lab. frame
          convert to GeV in cm. frame
          (assuming proton at rest as target AND
          assuming no pT, i.e. shower frame-z is aligned with hadron-int-frame-z)
        */
        // total energy: E_beam + E_target
        // in lab. frame: E_beam + m_target*c**2
        EnergyType E = p.GetEnergy();
        EnergyType Etot = E + Etarget;
        // total momentum
        MomentumVector Ptot = p.GetMomentum();
        // invariant mass, i.e. cm. energy
        EnergyType Ecm = sqrt(Etot * Etot - Ptot.squaredNorm());
        /*
         get transformation between Stack-frame and SibStack-frame
         for EAS Stack-frame is lab. frame, could be different for CRMC-mode
         the transformation should be derived from the input momenta
       */
        const double gamma = Etot / Ecm;
        const auto gambet = Ptot / Ecm;

        std::cout << "Interaction: "
                  << " DoDiscrete: gamma:" << gamma << endl;
        std::cout << "Interaction: "
                  << " DoDiscrete: gambet:" << gambet.GetComponents() << endl;

        int kBeam = process::sibyll::ConvertToSibyllRaw(p.GetPID());

        std::cout << "Interaction: "
                  << " DoInteraction: E(GeV):" << E / 1_GeV
                  << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;
        if (E < 8.5_GeV || Ecm < 10_GeV) {
          std::cout << "Interaction: "
                    << " DoInteraction: should have dropped particle.." << std::endl;
          // p.Delete(); delete later... different process
        } else {
          fCount++;
          // Sibyll does not know about units..
          const double sqs = Ecm / 1_GeV;
          // running sibyll, filling stack
          sibyll_(kBeam, kTarget, sqs);
          // running decays
          // setTrackedParticlesStable();
          decsib_();
          // print final state
          int print_unit = 6;
          sib_list_(print_unit);

          // delete current particle
          p.Delete();

          // add particles from sibyll to stack
          // link to sibyll stack
          // here we need to pass projectile momentum and energy to define the local
          // sibyll frame and the boosts to the lab. frame
          SibStack ss;

          // momentum array in Sibyll
          MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
          EnergyType E_final = 0_GeV, Ecm_final = 0_GeV;
          for (auto& psib : ss) {

            // skip particles that have decayed in Sibyll
            if (psib.HasDecayed()) continue;

            // transform energy to lab. frame, primitve
            // compute beta_vec * p_vec
            // arbitrary Lorentz transformation based on sibyll routines
            const auto gammaBetaComponents = gambet.GetComponents();
            // FOR NOW: fill vector in sibCS and then rotate into rootCS
            // can be done in SibStack by passing sibCS
            // get momentum vector in sibyllCS
            const auto pSibyllComponentsSibCS = psib.GetMomentum().GetComponents();
            // temporary vector in sibyllCS
            auto SibVector = MomentumVector(sibyllCS, pSibyllComponentsSibCS);
            // rotatate to rootCS
            const auto pSibyllComponents = SibVector.GetComponents(rootCS);
            // boost to lab. frame
            hep::EnergyType en_lab = 0. * 1_GeV;
            hep::MomentumType p_lab_components[3];
            en_lab = psib.GetEnergy() * gamma;
            hep::EnergyType pnorm = 0. * 1_GeV;
            for (int j = 0; j < 3; ++j)
              pnorm += (pSibyllComponents[j] * gammaBetaComponents[j]) / (gamma + 1.);
            pnorm += psib.GetEnergy();

            for (int j = 0; j < 3; ++j) {
              p_lab_components[j] =
                  pSibyllComponents[j] - (-1) * pnorm * gammaBetaComponents[j];
              en_lab -= (-1) * pSibyllComponents[j] * gammaBetaComponents[j];
            }

            // add to corsika stack
            auto pnew = s.NewParticle();
            pnew.SetEnergy(en_lab);
            pnew.SetPID(process::sibyll::ConvertFromSibyll(psib.GetPID()));
            corsika::geometry::QuantityVector<energy_hep_d> p_lab_c{
                p_lab_components[0], p_lab_components[1], p_lab_components[2]};
            pnew.SetMomentum(MomentumVector(rootCS, p_lab_c));
            pnew.SetPosition(pOrig);
            pnew.SetTime(tOrig);
            Plab_final += pnew.GetMomentum();
            E_final += en_lab;
            Ecm_final += psib.GetEnergy();
          }
          std::cout << "conservation (all GeV): E_final=" << E_final / 1_GeV
                    << ", Ecm_final=" << Ecm_final / 1_GeV
                    << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents()
                    << std::endl;
        }
      }
      return process::EProcessReturn::eOk;
    }
  };

} // namespace corsika::process::sibyll

#endif
