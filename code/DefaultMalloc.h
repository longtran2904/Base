/* date = August 7th 2022 1:30 am */

#ifndef _DEFAULT_MALLOC_H
#define _DEFAULT_MALLOC_H

#include <stdlib.h>

#if !defined(ReserveMem)
#define ReserveMem malloc
#endif
#if !defined(CommitMem)
#define CommitMem ChangeMemoryNoOp
#endif
#if !defined(DecommitMem)
#define DecommitMem ChangeMemoryNoOp
#endif
#if !defined(ReleaseMem)
#define ReleaseMem free
#endif

#endif //_DEFAULT_MALLOC_H
