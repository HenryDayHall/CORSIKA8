/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

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
