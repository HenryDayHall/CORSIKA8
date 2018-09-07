/**
   @File Logger.h
 */

#ifndef _include_logger_h_
#define _include_logger_h_

#include <iosfwd>
#include <string>
#include <sstream>
#include <typeinfo>

#include <boost/format.hpp>

#include <fwk/MessageOn.h>
#include <fwk/MessageOff.h>
#include <fwk/Sink.h>
#include <fwk/NoSink.h>
#include <fwk/BufferedSink.h>


using namespace std;
using namespace boost;

/**
   Everything around logfile generation and text output.
*/

namespace fwk {
  
  /**
     Defines one stream to accept messages, and to wrote those into
     TSink.  The helper class MessageOn will convert input at
     compile-time into message strings. The helper class MessageOff,
     will just do nothing and will be optimized away at compile time.
  */
  template<typename MSG=MessageOn, typename TSink=sink::NoSink> 
  class Logger : private MSG {
    
    using MSG::Message;
    
  public:
    // Logger() : fName("") {}
    Logger(const std::string color, const std::string name, TSink& sink) : fSink(sink), fName(color+"["+name+"]\033[39m ")  {} 
    ~Logger() { fSink.Close(); }    
    // Logger(const Logger&) = delete;

    /**
       Function to add string-concatenation of all inputs to output
       sink.
     */
    template<typename ... Strings>
    void Log(const Strings&... inputs) {
      fSink << MSG::Message(inputs...);
    }
    
    const std::string& GetName() const { return fName; }
    
  private:
    TSink& fSink;
    std::string fName;
  };

} // end namesapce 

#define LOG(__LOGGER,...)                                               \
  __LOGGER.Log(__LOGGER.GetName(), __FILE__,":", __LINE__, " (", __func__, ") -> ", ##__VA_ARGS__);

#endif
 
