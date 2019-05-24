/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/coast/ParticleConversion.h>
#include <corsika/particles/ParticleProperties.h>

#include <exception>
#include <iostream>
#include <sstream>

using namespace std;

namespace corsika::coast {

  /**
     Convert particle code, and check if it does exists. Throw exeption, if not!
   */

  corsika::particles::Code ConvertFromCoast(CoastCode pCode) {
    if (coast2corsika.count(pCode) == 0) {
      ostringstream err;
      err << "corsika::coast::ConvertFromCoast CoastCode does not exists="
          << static_cast<CoastCodeIntType>(pCode) << endl;
      cout << err.str() << endl;
      throw std::runtime_error(err.str());
    }
    return coast2corsika.find(pCode)->second;
  }

} // namespace corsika::coast
