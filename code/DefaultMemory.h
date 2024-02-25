/* date = July 30th 2022 6:08 pm */

#ifndef _DEFAULT_MEMORY_H
#define _DEFAULT_MEMORY_H

#if !defined(MemReserve)
#define MemReserve OSReserve
#endif
#if !defined(MemCommit)
#define MemCommit OSCommit
#endif
#if !defined(MemDecommit)
#define MemDecommit OSDecommit
#endif
#if !defined(MemRelease)
#define MemRelease OSRelease
#endif

#endif //_DEFAULT_MEMORY_H
