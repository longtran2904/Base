/* date = August 7th 2022 1:30 am */

#ifndef _DEFAULT_CRT_H
#define _DEFAULT_CRT_H

#include <stdlib.h>
#include <stdio.h>

#ifndef MemReserve
#define MemReserve malloc
#endif
#ifndef MemCommit
#define MemCommit ChangeMemoryNoOp
#endif
#ifndef MemDecommit
#define MemDecommit ChangeMemoryNoOp
#endif
#ifndef MemRelease
#define MemRelease free
#endif

#ifndef PrintOut
#define PrintOut(str) fprintf(stdout, "%.*s", StrExpand(str))
#endif
#ifndef PrintErr
#define PrintErr(str) fprintf(stderr, "%.*s", StrExpand(str))
#endif

#endif //_DEFAULT_CRT_H
