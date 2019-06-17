/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/coast/COASTProcess.h>
#include <corsika/coast/COASTStack.h>

#include <iostream>

using namespace std;

namespace corsika::coast {

  /**
     the init function is called during the start of corsika.
   */
  void COASTProcess::Init() {
    cout << "************* Greetings from CORSIKA8 *****************" << endl;
  }

  /**
     the docontinous function is called for each tracking step in
     corsika. Take care: you cannot modify the particle, the track or
     the stack from here (docontinuous) inside corisika7. In corsika8
     you will be able to do that.
   */
  corsika::process::EProcessReturn COASTProcess::DoContinuous(const Particle& p,
                                                              const Track& t,
                                                              const Stack&) {
    using namespace corsika::units::si;
    auto const start = t.GetPosition(0).GetCoordinates();
    auto const delta = t.GetPosition(1).GetCoordinates() - start;
    auto const name = corsika::particles::GetName(p.GetPID());
    cout << "CORSIKA8: particle=" << name << ", pos=" << start
         << " track-l=" << delta.norm() << ", track-t=" << t.GetDuration() << endl;
    return corsika::process::EProcessReturn::eOk;
  }

} // namespace corsika::coast
