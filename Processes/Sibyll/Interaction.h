#ifndef _corsika_process_sibyll_interaction_h_
#define _corsika_process_sibyll_interaction_h_

#include <corsika/process/InteractionProcess.h>

//#include <corsika/setup/SetupStack.h>
//#include <corsika/setup/SetupTrajectory.h>

#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>
#include <corsika/process/sibyll/sibyll2.3c.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika;
using namespace corsika::process::sibyll;
using namespace corsika::units::si;

namespace corsika::process::sibyll {

  // template <typename Stack, typename Track>
  // template <typename Stack>
  class Interaction
      : public corsika::process::InteractionProcess<Interaction> { // <Stack,Track>> {

    // typedef typename Stack::ParticleType Particle;
    // typedef typename corsika::setup::Stack::ParticleType Particle;
    // typedef corsika::setup::Trajectory Track;

  public:
    Interaction() {}
    ~Interaction() {}

    void Init() {
      corsika::random::RNGManager& rmng = corsika::random::RNGManager::GetInstance();
      rmng.RegisterRandomStream("s_rndm");

      // test random number generator
      std::cout << "Interaction: "
                << " test sequence of random numbers." << std::endl;
      int a = 0;
      for (int i = 0; i < 8; ++i) std::cout << i << " " << s_rndm_(a) << std::endl;

      // initialize Sibyll
      sibyll_ini_();
    }

    // void setTrackedParticlesStable();

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle& p, Track&) const {

      // coordinate system, get global frame of reference
      CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();

      const particles::Code corsikaBeamId = p.GetPID();

      // beam particles for sibyll : 1, 2, 3 for p, pi, k
      // read from cross section code table
      int kBeam = process::sibyll::GetSibyllXSCode(corsikaBeamId);

      bool kInteraction = process::sibyll::CanInteract(corsikaBeamId);

      /*
         the target should be defined by the Environment,
         ideally as full particle object so that the four momenta
         and the boosts can be defined..
       */
      // target nuclei: A < 18
      // FOR NOW: assume target is oxygen
      int kTarget = 16;

      EnergyType Etot = p.GetEnergy() + kTarget * corsika::particles::Proton::GetMass();
      super_stupid::MomentumVector Ptot(rootCS, {0.0_Ns, 0.0_Ns, 0.0_Ns});
      // FOR NOW: assume target is at rest
      super_stupid::MomentumVector pTarget(rootCS, {0.0_Ns, 0.0_Ns, 0.0_Ns});
      Ptot += p.GetMomentum();
      Ptot += pTarget;
      // calculate cm. energy
      EnergyType sqs = sqrt(Etot * Etot - Ptot.squaredNorm() * constants::cSquared);
      double Ecm = sqs / 1_GeV;

      std::cout << "Interaction: "
                << "MinStep: input en: " << p.GetEnergy() / 1_GeV << endl
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
        CrossSectionType sig = prodCrossSection * 1_mbarn;
        std::cout << "Interaction: "
                  << "MinStep: CrossSection (mb): " << sig / 1_mbarn << std::endl;

        const MassType nucleon_mass =
            0.93827_GeV / corsika::units::si::constants::cSquared;
        std::cout << "Interaction: "
                  << "nucleon mass " << nucleon_mass << std::endl;
        // calculate interaction length in medium
        GrammageType int_length = kTarget * nucleon_mass / sig;
        // pick random step lenth
        std::cout << "Interaction: "
                  << "interaction length (g/cm2): " << int_length << std::endl;

        return int_length;
      } else {
        return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
      }
    }

    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle& p, Stack& s) const {
      cout << "ProcessSibyll: "
           << "DoInteraction: " << p.GetPID() << " interaction? "
           << process::sibyll::CanInteract(p.GetPID()) << endl;
      if (process::sibyll::CanInteract(p.GetPID())) {
        cout << "defining coordinates" << endl;
        // coordinate system, get global frame of reference
        CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();

        QuantityVector<length_d> const coordinates{0_m, 0_m, 0_m};
        Point pOrig(rootCS, coordinates);

        /*
           the target should be defined by the Environment,
           ideally as full particle object so that the four momenta
           and the boosts can be defined..

           here we need: GetTargetMassNumber() or GetTargetPID()??
                         GetTargetMomentum() (zero in EAS)
        */
        // FOR NOW: set target to proton
        int kTarget = 1; // env.GetTargetParticle().GetPID();

        cout << "defining target momentum.." << endl;
        // FOR NOW: target is always at rest
        const EnergyType Etarget = 0. * 1_GeV + corsika::particles::Proton::GetMass();
        const auto pTarget = super_stupid::MomentumVector(
            rootCS, 0. * 1_GeV / constants::c, 0. * 1_GeV / constants::c,
            0. * 1_GeV / constants::c);
        cout << "target momentum (GeV/c): "
             << pTarget.GetComponents() / 1_GeV * constants::c << endl;
        cout << "beam momentum (GeV/c): "
             << p.GetMomentum().GetComponents() / 1_GeV * constants::c << endl;

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
        super_stupid::MomentumVector Ptot = p.GetMomentum(); // + pTarget;
        // invariant mass, i.e. cm. energy
        EnergyType Ecm =
            sqrt(Etot * Etot - Ptot.squaredNorm() *
                                   constants::cSquared); // sqrt( 2. * E * 0.93827_GeV );
        /*
         get transformation between Stack-frame and SibStack-frame
         for EAS Stack-frame is lab. frame, could be different for CRMC-mode
         the transformation should be derived from the input momenta
       */
        const double gamma = Etot / Ecm;
        const auto gambet = Ptot / (Ecm / constants::c);

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
                    << " DoInteraction: dropping particle.." << std::endl;
          p.Delete();
        } else {
          // Sibyll does not know about units..
          double sqs = Ecm / 1_GeV;
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
          SibStack ss;

          // SibStack does not know about momentum yet so we need counter to access
          // momentum array in Sibyll
          int i = -1;
          super_stupid::MomentumVector Ptot_final(rootCS, {0.0_Ns, 0.0_Ns, 0.0_Ns});
          for (auto& psib : ss) {
            ++i;
            // skip particles that have decayed in Sibyll
            if (abs(s_plist_.llist[i]) > 100) continue;

            // transform energy to lab. frame, primitve
            // compute beta_vec * p_vec
            // arbitrary Lorentz transformation based on sibyll routines
            const auto gammaBetaComponents = gambet.GetComponents();
            const auto pSibyllComponents = psib.GetMomentum().GetComponents();
            EnergyType en_lab = 0. * 1_GeV;
            MomentumType p_lab_components[3];
            en_lab = psib.GetEnergy() * gamma;
            EnergyType pnorm = 0. * 1_GeV;
            for (int j = 0; j < 3; ++j)
              pnorm += (pSibyllComponents[j] * gammaBetaComponents[j] * constants::c) /
                       (gamma + 1.);
            pnorm += psib.GetEnergy();

            for (int j = 0; j < 3; ++j) {
              p_lab_components[j] = pSibyllComponents[j] -
                                    (-1) * pnorm * gammaBetaComponents[j] / constants::c;
              en_lab -=
                  (-1) * pSibyllComponents[j] * gammaBetaComponents[j] * constants::c;
            }

            // add to corsika stack
            auto pnew = s.NewParticle();
            pnew.SetEnergy(en_lab);
            pnew.SetPID(process::sibyll::ConvertFromSibyll(psib.GetPID()));

            corsika::geometry::QuantityVector<momentum_d> p_lab_c{
                p_lab_components[0], p_lab_components[1], p_lab_components[2]};
            pnew.SetMomentum(super_stupid::MomentumVector(rootCS, p_lab_c));
            Ptot_final += pnew.GetMomentum();
          }
          // cout << "tot. momentum final (GeV/c): " << Ptot_final.GetComponents() / 1_GeV
          // * constants::c << endl;
        }
      }
      return process::EProcessReturn::eOk;
    }
  };

} // namespace corsika::process::sibyll

#endif
