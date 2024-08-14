/* date = July 5th 2024 2:21 am */

#ifndef _LONG_M_D_H
#define _LONG_M_D_H

//- long: Base Types
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

//- long: String Types
typedef String String8;
typedef StringList String8List;

enum
{
    StringMatchFlags_CaseInsensitive  = (1 << 0),
    StringMatchFlags_SlashInsensitive = (1 << 1),
    StringMatchFlags_RightSideSloppy  = (1 << 2),
};

StaticAssert(StringMatchFlags_CaseInsensitive  == MatchStr_IgnoreCase , CheckStringMatchFlags);
StaticAssert(StringMatchFlags_SlashInsensitive == MatchStr_IgnoreSlash, CheckStringMatchFlags);
StaticAssert(StringMatchFlags_RightSideSloppy  == MatchStr_RightSloppy, CheckStringMatchFlags);

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

//- long: Arena Types
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

//- long: Helper Functions
function MD_ParseResult MD_ParseText(Arena* arena, String filename, String text)
{
    MD_TokenizeResult tokenize = md_tokenize_from_text(arena, text);
    MD_ParseResult result = md_parse_from_text_tokens(arena, filename, text, tokenize.tokens);
    return result;
}

#endif //_LONG_M_D_H
