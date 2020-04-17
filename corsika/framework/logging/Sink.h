/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Sink_h_
#define _include_Sink_h_

namespace corsika::logging {

  /**
     a sink for the logger must implement the two functions
     operator<<(const std::string&)
     and
     Close()

     See example: NoSink
   */

  namespace sink {

    /**
       Definition of Sink for log output.
    */
    template <typename TStream>
    class Sink {
    public:
      Sink(TStream& out)
          : fOutput(out) {}
      void operator<<(const std::string& msg) { fOutput << msg; }
      void Close() {}

    private:
      TStream& fOutput;
    };

    typedef Sink<std::ostream> SinkStream;

  } // namespace sink
} // namespace corsika::logging

#endif
