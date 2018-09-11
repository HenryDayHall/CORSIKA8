#ifndef _Physics_NullModel_NullModel_h_
#define _Physics_NullModel_NullModel_h_

namespace corsika::process {

  namespace null_model {

    class NullModel {

    public:
      NullModel();
      ~NullModel();

      void init();
      void run();
      double GetStepLength();
    };

  } // namespace null_model

} // namespace corsika::process

#endif
