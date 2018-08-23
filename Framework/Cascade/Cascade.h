#ifndef _include_Cascade_h_
#define _include_Cascade_h_

namespace cascade {

  template<typename Processes, typename Trajectory, typename Stack>
  class Cascade {
    
  public:
    Cascade();

    void Init();
    void Run();
    void Step(Particle& particle);

  private:
    Stack fStack;
    Processes fProcesseList;
    
  };
  
}

#endif
