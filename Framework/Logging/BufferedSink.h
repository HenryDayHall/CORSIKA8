#ifndef _include_BufferedSink_h_
#define _include_BufferedSink_h_

namespace logger {

  namespace sink {
    

    /**
       Output buffer template. NoBuffer does nothingk.
    */
    /*
    struct NoBuffer {
      inline bool Test(const std::string&) const { return false; }
      inline std::string GetString() const { return std::string(""); }
      inline void Clear() {}
      inline void Add(const std::string&) {}
    };
    */
    /**
       Output buffer template. StdBuffer records fSize characters in
       local memeory before passing it on to further output stages. 
    */
    
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
    
    
    /**
       Definition of Sink for log output. 
    */  
    template<typename TStream, typename TBuffer=StdBuffer> 
    class BufferedSink {
    public:
    BufferedSink(TStream& out, TBuffer buffer = {} ) : fOutput(out), fBuffer(std::move(buffer)) {} 
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
    
    typedef BufferedSink<std::ostream, StdBuffer> BufferedSinkStream;

  }// end namespace
} // end namespace
  
#endif
