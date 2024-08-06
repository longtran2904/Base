/* date = July 5th 2024 2:21 am */

#ifndef _LONG_M_D_H
#define _LONG_M_D_H

#if 0
#define MD_DEFAULT_ARENA 0
#define MD_IMPL_Arena Arena
#define MD_IMPL_ArenaMinPos 64
#define MD_IMPL_ArenaAlloc     ArenaMake
#define MD_IMPL_ArenaRelease   ArenaRelease
#define MD_IMPL_ArenaGetPos    ArenaCurrPos
#define MD_IMPL_ArenaPush      ArenaPush
#define MD_IMPL_ArenaPopTo     ArenaPopTo
#define MD_IMPL_ArenaSetAutoAlign ArenaAlign

#define MD_DEFAULT_SCRATCH 0
function Arena* Long_GetMDScratch(Arena** conflicts, u64 count) { return GetScratch(conflicts, count).arena; }
#define MD_IMPL_GetScratch Long_GetMDScratch

#define MD_DEFAULT_SPRINTF 0
#define MD_IMPL_Vsnprintf stbsp_vsnprintf

#define MD_STR(string) (MD_String8){ .str = (string).str, .size = (string).size }

MSVC(WarnDisable(6250 28182))
#include "md\md.h"
#include "md\md.c"
MSVC(WarnEnable(6250 28182))
#else

//- Base Types
typedef i8 S8;
typedef i16 S16;
typedef i32 S32;
typedef i64 S64;
typedef u8 U8;
typedef u16 U16;
typedef u32 U32;
typedef u64 U64;
typedef b32 B32;

typedef union r1u64 Rng1U64;

#define read_only readonly

//- String Types
typedef String String8;
typedef StringList String8List;

typedef u32 StringMatchFlags;
enum
{
    StringMatchFlag_CaseInsensitive  = (1 << 0),
    StringMatchFlag_RightSideSloppy  = (1 << 1),
    StringMatchFlag_SlashInsensitive = (1 << 2),
};

#define str8_lit StrLit
#define push_str8fv StrPushfv
#define push_str8f  StrPushf
#define str8_list_pushf StrListPushf
#define str8_list_push  StrListPush

#define str8_match StrCompare
#define str8_chop StrChop
#define str8_skip StrSkip
#define str8_substr(str, range) Substr((str), (range).min, (range).max)

read_only global U8 utf8_class[32] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

//- Arena Types
typedef TempArena Temp;

#define MemoryCopy CopyMem
#define MemoryCopyStruct CopyStruct
#define MemoryZeroStruct ZeroStruct

#define push_array PushArray
#define push_array_no_zero PushArrayNZ

#define scratch_begin GetScratch
#define scratch_end TempEnd

WarnPush(0)
WarnDisable("-Wall")
WarnDisable("-Wextra")
#include "mdesk\mdesk.h"
#include "mdesk\mdesk.c"
WarnPop()
#endif

#endif //_LONG_M_D_H
