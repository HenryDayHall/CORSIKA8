/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/FourVector.h>
#include <corsika/process/on_shell_check/OnShellCheck.h>

using namespace std;

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units::si;
using namespace corsika::particles;
using namespace corsika::setup;

namespace corsika::process {
  namespace on_shell_check {

    void OnShellCheck::Init() {
      std::cout << "OnShellCheck: mass tolerance is set to " << mass_tolerance_ * 100
                << "%" << endl
                << "              energy tolerance is set to " << energy_tolerance_ * 100
                << "%" << std::endl;
    }

    EProcessReturn OnShellCheck::DoSecondaries(corsika::setup::StackView& vS) {
      for (auto& p : vS) {
        auto const pid = p.GetPID();
        if (!particles::IsHadron(pid) || particles::IsNucleus(pid)) continue;
        auto const e_original = p.GetEnergy();
        auto const p_original = p.GetMomentum();
        auto const Plab = corsika::geometry::FourVector(e_original, p_original);
        auto const m_kinetic = Plab.GetNorm();
        auto const m_corsika = particles::GetMass(pid);
        auto const m_err_abs = abs(m_kinetic - m_corsika);
        if (m_err_abs >= mass_tolerance_ * m_corsika) {
          const HEPEnergyType e_shifted =
              sqrt(p_original.GetSquaredNorm() + m_corsika * m_corsika);
          auto const e_shift_relative = (e_shifted / e_original - 1);
          count_ = count_ + 1;
          average_shift_ += abs(e_shift_relative);
          if (abs(e_shift_relative) > max_shift_) max_shift_ = abs(e_shift_relative);
          std::cout << "OnShellCheck: shift particle mass for " << pid << std::endl
                    << std::setw(40) << std::setfill(' ')
                    << "corsika mass (GeV): " << m_corsika / 1_GeV << std::endl
                    << std::setw(40) << std::setfill(' ')
                    << "kinetic mass (GeV): " << m_kinetic / 1_GeV << std::endl
                    << std::setw(40) << std::setfill(' ')
                    << "m_kin-m_cor (GeV): " << m_err_abs / 1_GeV << std::endl
                    << std::setw(40) << std::setfill(' ')
                    << "mass tolerance (GeV): " << (m_corsika * mass_tolerance_) / 1_GeV
                    << std::endl;
          /*
            For now we warn if the necessary shift is larger than 1%.
            we could promote this to an error.
          */
          if (abs(e_shift_relative) > energy_tolerance_) {
            std::cout << "OnShellCheck: warning! shifted particle energy by "
                      << e_shift_relative * 100 << " %" << std::endl;
            if (throw_error_)
              throw std::runtime_error(
                  "OnShellCheck: error! shifted energy by large amount!");
          }

          // reset energy
          p.SetEnergy(e_shifted);
        } else
          std::cout << "OnShellCheck: particle mass for " << pid << " OK" << std::endl;
      }
      return EProcessReturn::eOk;
    }
  } // namespace on_shell_check
} // namespace corsika::process
