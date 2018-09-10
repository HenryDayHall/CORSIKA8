#ifndef _include_NoSink_h_
#define _include_NoSink_h_

namespace corsika::logging {

  namespace sink {

    struct NoSink {
      inline void operator<<(const std::string&) {}
      inline void Close() {}
    };

  }// end namespace
} // end namespace
  
#endif
