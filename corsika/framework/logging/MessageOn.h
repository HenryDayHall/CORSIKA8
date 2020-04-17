/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_MessageOn_h_
#define _include_MessageOn_h_

namespace corsika::logging {

  /**
     Helper class to convert all input arguments of MessageOn::Message
     into string-concatenated version and return this as string.
  */
  class MessageOn {
  protected:
    std::string Message() { return "\n"; }

    template <typename First, typename... Strings>
    std::string Message(const First& arg, const Strings&... rest) {
      std::ostringstream ss;
      ss << arg << Message(rest...);
      return ss.str();
    }

    template <typename... Strings>
    std::string Message(const int& arg, const Strings&... rest) {
      return std::to_string(arg) + Message(rest...);
    }

    template <typename... Strings>
    std::string Message(const double& arg, const Strings&... rest) {
      return std::to_string(arg) + Message(rest...);
    }

    template <typename... Strings>
    std::string Message(char const* arg, const Strings&... rest) {
      return std::string(arg) + Message(rest...);
    }

    template <typename... Strings>
    std::string Message(const std::string& arg, const Strings&... rest) {
      return arg + Message(rest...);
    }

    // ----------------------
    // boost format
    template <typename... Strings>
    std::string Message(const boost::format& fmt, const Strings&... rest) {
      boost::format FMT(fmt);
      return bformat(FMT, rest...);
    }

    template <typename Arg, typename... Strings>
    std::string bformat(boost::format& fmt, const Arg& arg, const Strings&... rest) {
      fmt % arg;
      return bformat(fmt, rest...);
    }

    std::string bformat(boost::format& fmt) { return fmt.str() + "\n"; }
  };

} // namespace corsika::logging

#endif
