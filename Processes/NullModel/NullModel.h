#ifndef _Physics_NullModel_NullModel_h_
#define _Physics_NullModel_NullModel_h_

namespace physics {

  namespace processes {

    class NullModel {

    public:
      NullModel();
      ~NullModel();

      void init();
      void run();
      double GetStepLength();
    };
    
  }
}

#endif

