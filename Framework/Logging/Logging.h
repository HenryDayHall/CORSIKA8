#ifndef _include_logging_h_
#define _include_logging_h_

#include <logger.h>

#include <map>
#include <string>
#include <any>

class Logging {
  
  Logging() {}
  
 public:
  
  static Logging& GetInstance() { static Logging fgLog; return fgLog; }

  template<typename TLogger>
    void AddLogger(const std::string& name, const TLogger& logger) { fLoggers[name] = logger; }

  auto& GetLogger(const std::string& name) { return fLoggers[name]; }
  
 private:
  std::map<std::string, std::any> fLoggers;
};

#endif
