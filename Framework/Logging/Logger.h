#ifndef _include_logger_h_
#define _include_logger_h_

#include <string>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include <fstream>

#include <boost/format.hpp>
 
using namespace std;
using namespace boost;


class MessageOff {
 protected:
  template<typename First, typename ... Strings> std::string message(const First& arg, const Strings&... rest) {
    return "";
  }
};
  
class messageconst {
 protected:
  std::string message() { return "\n"; }

  template<typename First, typename ... Strings> std::string message(const First& arg, const Strings&... rest) {
    std::ostringstream ss;
    ss << arg << message(rest...);
    return ss.str();
  }
  
  template<typename ... Strings> std::string message(const int& arg, const Strings&... rest) {
    return std::to_string(arg) + message(rest...);
  }

  template<typename ... Strings> std::string message(const double& arg, const Strings&... rest) {
    return std::to_string(arg) + message(rest...);
  }
  
  template<typename ... Strings> std::string message(char const * arg, const Strings&... rest) {
    return std::string(arg) + message(rest...);
  }
  
  template<typename ... Strings> std::string message(const std::string& arg, const Strings&... rest) {
    return arg + message(rest...);
  }
  
  // ----------------------
  // boost format
  template<typename ... Strings> std::string message(const boost::format& fmt, const Strings&... rest) {
    boost::format FMT(fmt);
    return bformat(FMT, rest...);
  }
  
  template<typename Arg, typename ... Strings> std::string bformat(boost::format& fmt, const Arg& arg, const Strings&... rest) {
    fmt % arg;
    return bformat(fmt, rest...);
  }

  std::string bformat(boost::format& fmt) { return fmt.str() + "\n"; }
};


struct NoBuffer {
  inline bool Test(const std::string&) const { return false; }
  inline std::string GetString() const { return std::string(""); }
  inline void Clear() {}
  inline void Add(const std::string&) {}
};

struct StdBuffer {
  StdBuffer(const int size) : fSize(size) {}
  inline bool Test(const std::string& s) { return int(fBuffer.tellp())+s.length() < fSize; }
  inline std::string GetString() const { return fBuffer.str(); }
  inline void Clear() { fBuffer.str(""); }
  inline void Add(const std::string& s) { fBuffer << s; }
private:
  int fSize;
  std::ostringstream fBuffer;
};


template<typename TStream, typename TBuffer=StdBuffer> 
class Sink {
 public:
  Sink(TStream& out, TBuffer buffer = {} ) : fOutput(out), fBuffer(std::move(buffer)) {} 
  void operator<<(const std::string& msg) {
    if (!fBuffer.Test(msg)) {
      fOutput << fBuffer.GetString();
      fBuffer.Clear();
    }
    if (!fBuffer.Test(msg))
      fOutput << msg;
    else 
      fBuffer.Add(msg);
  }
 void Close() { fOutput << fBuffer.GetString(); }
 private:
  TStream& fOutput;
  TBuffer fBuffer;
};



struct NoSink { inline void operator<<(const std::string&) {} inline void Close() {} };


template<typename TSink=NoSink,typename M=messageconst> 
class logger : private M {
  
  using M::message;

 public:
  // logger() : fName("") {}
  logger(const std::string color, const std::string name, TSink& sink) : fSink(sink), fName(color+"["+name+"]\033[39m ")  {} 
  ~logger() { fSink.Close(); }
  
  // logger(const logger&) = delete;
  
  template<typename ... Strings>
  void log(const Strings&... inputs) {
    fSink << M::message(inputs...);
  }

  const std::string& GetName() const { return fName; }
  
 private:
  TSink& fSink;
  std::string fName;
};



#define LOG(__LOGGER,...) \
  __LOGGER.log(__LOGGER.GetName(), __FILE__,":", __LINE__, " (", __func__, ") -> ", ##__VA_ARGS__);




#endif
 
