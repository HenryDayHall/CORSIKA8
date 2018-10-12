#ifndef _include_corsika_baseprocess_h_
#define _include_corsika_baseprocess_h_

#include <corsika/process/ProcessReturn.h> // for convenience

namespace corsika::process {

  /**
     \class BaseProcess

     The structural base type of a process object in a
     ProcessSequence. Both, the ProcessSequence and all its elements
     are of type BaseProcess<T>

   */

  template <typename derived>
  struct BaseProcess {
    derived& GetRef() { return static_cast<derived&>(*this); }
    const derived& GetRef() const { return static_cast<const derived&>(*this); }
  };

}

#endif
