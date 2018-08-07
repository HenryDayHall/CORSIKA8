#include <Logging/Logger.h>

#include <string>
#include <iostream>

#include <boost/format.hpp>

using namespace std;

int
main()
{
  {
    cout << "writing to \"another.log\"" << endl;
    ofstream logfile("another.log");
    typedef Sink<ofstream, StdBuffer> SinkFile;
    SinkFile sink(logfile, StdBuffer(10000));
    logger<SinkFile, messageconst> info("\033[32m", "info", sink);
    logger<SinkFile, messageconst> err("\033[31m", "error", sink);
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
    NoSink off;
    logger<NoSink, MessageOff> info("", "", off);
    
    for (int i=0; i<100000; ++i) {
      LOG(info, "irgendwas", string("and more"), boost::format("error: %i message: %s. done."), i, "stupido", "a-number:", 8.99, "ENDE" );
    }
  }
  
  return 0;
}
