/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/**
   @File Logger.h

   Everything around logfile generation and text output.
 */

#ifndef _include_logger_h_
#define _include_logger_h_

#include <iosfwd>
#include <sstream>
#include <string>
#include <typeinfo>

#include <boost/format.hpp>

#include <corsika/logging/BufferedSink.h>
#include <corsika/logging/MessageOff.h>
#include <corsika/logging/MessageOn.h>
#include <corsika/logging/NoSink.h>
#include <corsika/logging/Sink.h>

namespace corsika::logging {

  /**
     @class Logger

     Defines one stream to accept messages, and to wrote those into
     TSink.  The helper class MessageOn will convert input at
     compile-time into message strings. The helper class MessageOff,
     will just do nothing and will be optimized away at compile time.
  */
  template <typename MSG = MessageOn, typename TSink = sink::NoSink>
  class Logger : private MSG {

    using MSG::Message;

  public:
    // Logger() : fName("") {}
    Logger(const std::string color, const std::string name, TSink& sink)
        : fSink(sink)
        , fName(color + "[" + name + "]\033[39m ") {}
    ~Logger() { fSink.Close(); }
    // Logger(const Logger&) = delete;

    /**
       Function to add string-concatenation of all inputs to output
       sink.
     */
    template <typename... Strings>
    void Log(const Strings&... inputs) {
      fSink << MSG::Message(inputs...);
    }

    const std::string& GetName() const { return fName; }

  private:
    TSink& fSink;
    std::string fName;
  };

} // namespace corsika::logging

/**
 * @def LOG(...)
 *
 * This is the main interface to the logging facilities. If Logger
 * object are defined (e.g. log1) use as
 * @example LOG(log1, "var1=", variable1int, "var2=", variabl2double)
 * for arbitrary long sequence
 * of arguments. This may also include boost::format objects the
 * output is concatenated, if log1 is switched off at compile time,
 * the whole LOG command is optimized away by the compiler.
 */

#define LOG(__LOGGER, ...)                                                           \
  __LOGGER.Log(__LOGGER.GetName(), __FILE__, ":", __LINE__, " (", __func__, ") -> ", \
               __VA_ARGS__);

#endif
