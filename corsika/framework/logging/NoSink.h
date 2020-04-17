/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_NoSink_h_
#define _include_NoSink_h_

namespace corsika::logging {

  namespace sink {

    struct NoSink {
      inline void operator<<(const std::string&) {}
      inline void Close() {}
    };

  } // namespace sink
} // namespace corsika::logging

#endif
