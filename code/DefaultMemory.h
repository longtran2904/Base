/* date = July 30th 2022 6:08 pm */

#ifndef _DEFAULT_MEMORY_H
#define _DEFAULT_MEMORY_H

#if !defined(ReserveMem)
#define ReserveMem OSReserve
#endif
#if !defined(CommitMem)
#define CommitMem OSCommit
#endif
#if !defined(DecommitMem)
#define DecommitMem OSDecommit
#endif
#if !defined(ReleaseMem)
#define ReleaseMem OSRelease
#endif

#endif //_DEFAULT_MEMORY_H
