#ifndef _Processes_UrQMD_UrQMD_h
#define _Processes_UrQMD_UrQMD_h

extern "C" {
    void iniurqmd_();
    double ranf_(int*);
}

namespace corsika::process::UrQMD {
    class UrQMD {
    public:
        UrQMD();
    };
}

#endif
