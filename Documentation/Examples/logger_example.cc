#include <fwk/Logger.h>

#include <string>
#include <iostream>
#include <fstream>

#include <boost/format.hpp>

using namespace std;

int
main()
{
  {
    cout << "writing to \"another.log\"" << endl;
    ofstream logfile("another.log");
    fwk::sink::SinkStream unbuffered_sink(logfile);
    fwk::sink::BufferedSinkStream sink(logfile, fwk::sink::StdBuffer(10000));
    fwk::Logger<fwk::MessageOn, fwk::sink::BufferedSinkStream> info("\033[32m", "info", sink);
    fwk::Logger<fwk::MessageOn, fwk::sink::BufferedSinkStream> err("\033[31m", "error", sink);
    //logger<ostream,messageconst,StdBuffer> info(std::cout, StdBuffer(10000));
    
    /*
      Logging& logs = Logging::GetInstance();
      logs.AddLogger<>("info", info);
      auto& log_1 = logs.GetLogger("info"); // no so useful, since type of log_1 is std::any
    */
    
    for (int i=0; i<100000; ++i) {
      LOG(info, "irgendwas"," ", string("and more")," ", boost::format("error: %i message: %s. done."), i, "stupido");
      LOG(err, "Fehler");
    }
  }
  
  {
    fwk::sink::NoSink off;
    fwk::Logger<fwk::MessageOff> info("", "", off);
    
    for (int i=0; i<100000; ++i) {
      LOG(info, "irgendwas", string("and more"), boost::format("error: %i message: %s. done."), i, "stupido", "a-number:", 8.99, "ENDE" );
    }
  }
  
  return 0;
}
