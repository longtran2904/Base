/* date = August 7th 2022 1:30 am */

#ifndef _DEFAULT_MALLOC_H
#define _DEFAULT_MALLOC_H

#include <stdlib.h>

#if !defined(MemReserve)
#define MemReserve malloc
#endif
#if !defined(MemCommit)
#define MemCommit ChangeMemoryNoOp
#endif
#if !defined(MemDecommit)
#define MemDecommit ChangeMemoryNoOp
#endif
#if !defined(MemRelease)
#define MemRelease free
#endif

#endif //_DEFAULT_MALLOC_H
