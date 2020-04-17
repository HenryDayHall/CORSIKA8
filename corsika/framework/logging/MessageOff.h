/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_MessageOff_h_
#define _include_MessageOff_h_

namespace corsika::logging {

  /**
     Helper class to ignore all arguments to MessagesOn::Message and
     always return empty string "".
   */
  class MessageOff {
  protected:
    template <typename First, typename... Strings>
    std::string Message(const First&, const Strings&...) {
      return "";
    }
  };

} // namespace corsika::logging

#endif
