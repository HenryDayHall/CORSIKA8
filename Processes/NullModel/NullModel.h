
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _Physics_NullModel_NullModel_h_
#define _Physics_NullModel_NullModel_h_

namespace corsika::process {

  namespace null_model {

    class NullModel {

    public:
      NullModel();
      ~NullModel();

      void init();
      void run();
      double GetStepLength();
    };

  } // namespace null_model

} // namespace corsika::process

#endif
