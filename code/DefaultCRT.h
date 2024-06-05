/* date = August 7th 2022 1:30 am */

#ifndef _DEFAULT_CRT_H
#define _DEFAULT_CRT_H

#include <stdlib.h>
#include <stdio.h>

#ifndef MemReserve
#define MemReserve(size) malloc(size)
#endif
#ifndef MemCommit
#define MemCommit(ptr, size) 1
#endif
#ifndef MemDecommit
#define MemDecommit(ptr, size)
#endif
#ifndef MemRelease
#define MemRelease(ptr) free(ptr)
#endif

#ifndef PrintOut
#define PrintOut(str) fprintf(stdout, "%.*s", StrExpand(str))
#endif
#ifndef PrintErr
#define PrintErr(str) fprintf(stderr, "%.*s", StrExpand(str))
#endif

#endif //_DEFAULT_CRT_H
