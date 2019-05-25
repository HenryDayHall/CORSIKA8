/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/logging/Logger.h>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace corsika::logging;

int main() {
  {
    cout << "writing to \"another.log\"" << endl;
    ofstream logfile("another.log");
    sink::SinkStream unbuffered_sink(logfile);
    sink::BufferedSinkStream sink(logfile, sink::StdBuffer(10000));
    Logger<MessageOn, sink::BufferedSinkStream> info("\033[32m", "info", sink);
    Logger<MessageOn, sink::BufferedSinkStream> err("\033[31m", "error", sink);
    // logger<ostream,messageconst,StdBuffer> info(std::cout, StdBuffer(10000));

    /*
      Logging& logs = Logging::GetInstance();
      logs.AddLogger<>("info", info);
      auto& log_1 = logs.GetLogger("info"); // no so useful, since type of log_1 is
      std::any
    */

    for (int i = 0; i < 10000; ++i) {
      LOG(info, "irgendwas", " ", string("and more"), " ",
          boost::format("error: %i message: %s. done."), i, "stupido");
      LOG(err, "Fehler");
    }
  }

  {
    sink::NoSink off;
    Logger<MessageOff> info("", "", off);

    for (int i = 0; i < 10000; ++i) {
      LOG(info, "irgendwas", string("and more"),
          boost::format("error: %i message: %s. done."), i, "stupido", "a-number:", 8.99,
          "ENDE");
    }
  }

  return 0;
}
