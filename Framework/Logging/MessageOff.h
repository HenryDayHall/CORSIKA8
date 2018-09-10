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
    std::string Message(const First& arg, const Strings&... rest) {
      return "";
    }
  };

} // namespace corsika::logging

#endif
