#include "Base.h"
#include "Base.c"

i32 main(i32 argc, char** argv)
{
    ScratchBegin(scratch);
    
    DeferBlock(Outf("argc: %d\n", argc), Outf("\n"))
    {
        for (i32 i = 0; i < argc; ++i)
            Outf("argv[%d] = `%s`\n", i, argv[i]);
    }
    
    StringList args = OSSetArgs(argc, argv);
    CmdLine cmd = CmdLineFromList(scratch, &args);
    Outf("Name: %.*s\n\n", StrExpand(cmd.programName));
    
    String input = StrJoin(scratch, &cmd.inputs, .mid = StrLit("\n"));
    Outf("Inputs: %llu\n%.*s\n\n", cmd.inputs.nodeCount, StrExpand(input));
    
    Outf("Args: %llu\n", cmd.opts.count);
    for (CmdLineOpt* opt = cmd.opts.first; opt; opt = opt->next)
    {
        String value = {0};
        if (opt->values.nodeCount)
            value = StrJoin(scratch, &opt->values, .pre = StrLit(" = "), .mid = StrLit(", "));
        Outf("%.*s%.*s\n", StrExpand(opt->name), StrExpand(value));
    }
    
    ScratchEnd(scratch);
    return 0;
}
