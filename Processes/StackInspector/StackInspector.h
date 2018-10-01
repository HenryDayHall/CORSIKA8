#ifndef _Physics_StackInspector_StackInspector_h_
#define _Physics_StackInspector_StackInspector_h_

namespace corsika::process {

  namespace stack_inspector {

    class StackInspector {

    public:
      StackInspector();
      ~StackInspector();

      void init();
      void run();
      double GetStepLength();
    };

  } // namespace stack_inspector

} // namespace corsika::process

#endif
