#include "Base.h"
#include "Base.c"

i32 main(i32 argc, char** argv)
{
    ScratchBegin(scratch);
    
    // csv
    // json
    // mdesk
    // c/c++
    // header
    // Batch/shell
    // files - blank - comment - code
    
    StringList args = OSSetArgs(argc, argv);
    CmdLine cmd = CmdLineFromList(scratch, &args);
    DEBUG(cmd);
    
    ScratchEnd(scratch);
    return 0;
}
