#include "DefaultMemory.h"
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"

#include "MetaParser.h"
#include "MetaParser.c"

// GOAL:
// 1) Introspection (enum's name and type definition)
// 2) Code insertion (defer, closure, and CALLER_(FILE/LINE))
// 3) Table-Driven Code Generation
// 4) Compile Time Code Generation (@if, @assert, @type, @var, @run, @ensure, function overloading, default parameter)

// TODO:
// Clean up the API and compress thing out
// Improve PrintType function
// Calculate union size
// Parse function
// Parse expression, enum, and declaration with assignment
// Parse statement

i32 maxPrints = 0;
function void PrintType(MemArena* arena, MetaInfo* type, u32 padding, StringList* ignoreTypes, StringList* result)
{
    if (maxPrints > 100)
    {
        printf("PANIC!!!");
        Assert(0);
    }
    
    maxPrints++;
    //PushStrList(arena, result, PushStrf(arena, false, "\n"));
    if (type->name.size == 0)
        PushStrList(arena, result, PushStrf(arena, false, "%*.s<Anonymous>\n", padding, ""));
    else
        PushStrList(arena, result, PushStrf(arena, false, "%*.s[%.*s]\n", padding, "", StrExpand(type->name)));
    
    padding += 4;
    PushStrList(arena, result, PushStrf(arena, false, "%*.sType:  %s\n", padding, "", GetEnumName(MetaInfoKind, type->kind)));
    PushStrList(arena, result, PushStrf(arena, false, "%*.sFlags: %s\n", padding, "",
                                        GetFlagName(arena, MetaInfoFlag, type->flags)));
    PushStrList(arena, result, PushStrf(arena, false, "%*.sIndex: %d\n", padding, "", type->indexWithinTypeTable));
    PushStrList(arena, result, PushStrf(arena, false, "%*.sSize:  %d\n", padding, "", type->runtimeSize));
    PushStrList(arena, result, PushStrf(arena, false, "%*.sRefs:  ", padding, ""));
    
    u32 pd = 0;
    for (MetaInfo* ref = type->first; ref; ref = ref->next)
    {
        if (ref->first != type && ref->first && CompareStrList(ref->first->name, ignoreTypes, StringMatchFlag_NotEqual))
        {
            if (ref == type->first)
                PushStrList(arena, result, PushStrf(arena, false, "%llu\n", type->count));
            PrintType(arena, ref, padding + 7, ignoreTypes, result);
        }
        else
            PushStrList(arena, result, PushStrf(arena, false, "%*.s[%.*s: %s]\n", pd, "",
                                                StrExpand(ref->name), GetEnumName(MetaInfoKind, ref->kind)));
        
        if (IsRef(type->kind))
            break;
        pd = padding + 7;
    }
    
    PushStrList(arena, result, PushStrf(arena, false, "\n"));
    maxPrints--;
}

int main(void)
{
    InitOSMain(0, 0);
    BeginScratch(scratch);
    
    String fileName = StrLit("code/MetaTest.c");
    String file = ReadOSFile(scratch, fileName, true);
    file.size += 1; // include the end of file
    
    file = ReplaceStr(scratch, file, StrLit("\\\n"), StrLit(""), 0, 0);
    
    Lexer lexer = Lexing(file, fileName, scratch);
    Parser parser = Parsing(&lexer, scratch);
    
#define PRIMITIVE_TYPE(X) \
X(Int   , 1 , "i8"    ) \
X(Int   , 2 , "i16"   ) \
X(Int   , 4 , "i32"   ) \
X(Int   , 8 , "i64"   ) \
X(UInt  , 1 , "u8"    ) \
X(UInt  , 2 , "u16"   ) \
X(UInt  , 4 , "u32"   ) \
X(UInt  , 8 , "u64"   ) \
X(Float , 4 , "f32"   ) \
X(Float , 8 , "f64"   ) \
X(Bool  , 1 , "b8"    ) \
X(Bool  , 2 , "b16"   ) \
X(Bool  , 4 , "b32"   ) \
X(Bool  , 8 , "b64"   ) \
X(String, 16, "String") \
X(Void  , 0 , "void"  ) \
    
#define ADD_PRIMITIVE(t, s, n) AddMetaInfo(parser.table, 0, &(MetaInfo){ \
.kind = MetaInfoKind_##t, \
.flags = MetaInfoFlag_DoneCompiling, \
.runtimeSize = s, \
.name = StrLit(n) });
    PRIMITIVE_TYPE(ADD_PRIMITIVE);
#undef ADD_PRIMITIVE
    
#define PRIMITIVE_ARRAY(t, s, n) ConstStr(n),
    StringList ignoreTypes = StrList(scratch, ArrayExpand(String, PRIMITIVE_TYPE(PRIMITIVE_ARRAY)));
#undef PRIMITIVE_ARRAY
    
    MetaInfo* start = parser.table->last;
    b32 parsing = true;
    
    while (parsing && !parser.lexer->error)
    {
        Token token = GetToken(parser.lexer);
        if (token.type == MetaTokenType_EndOfStream)
        {
            token.str = StrLit("<End of File>");
            parsing = false;
        }
        else if (token.type == MetaTokenType_Keyword_typedef)
            ParseDeclaration(&parser, Declaration_Typedef, 0);
        else if (IsType(token.type))
            ParseDeclaration(&parser, Declaration_Type, 0);
        
        printf("Token(%u, %u)\n                 str  = %.*s\n                 type = %s\n", token.lineNumber, token.colNumber,
               StrExpand(token.str), GetEnumName(MetaTokenType, token.type));
    }
    
    StringList list = {0};
    for (MetaInfo* type = start->next; type; type = type->next)
        if (type->flags & MetaInfoFlag_DoneCompiling)
        PrintType(scratch, type, 0, &ignoreTypes, &list);
    
    String typeData = JoinStr(scratch, &list, 0, false);
    printf("%.*s", StrExpand(typeData));
    WriteOSFile(StrLit("code/generated/Types.txt"), typeData);
    
    for (MetaInfo* type = parser.table->first; type; type = type->next)
        printf("[%.*s: %s],\n", StrExpand(type->name), GetEnumName(MetaInfoKind, type->kind));
    
    for (StringNode* node = parser.lexer->errorList->first; node; node = node->next)
        printf("%.*s", StrExpand(node->string));
    
    EndScratch(scratch);
    return 0;
}
