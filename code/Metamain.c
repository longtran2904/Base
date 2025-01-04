#include "Base.h"
#include "Base.c"

#include "LongMD.h"
#include "Metagen.h"
#include "Metagen.c"

typedef enum MG_GenType MG_GenType;
enum MG_GenType
{
    MG_Gen_Null,
    
    MG_Gen_Enum,
    MG_Gen_Union,
    MG_Gen_Struct,
    
    MG_Gen_Text,
    MG_Gen_Embed,
    MG_Gen_Table,
    
    MG_Gen_Function,
    MG_Gen_All,
    
    MG_Gen_COUNT,
};

i32 main(i32 argc, char** argv)
{
    OSSetArgs(argc, argv);
    
    //- long: set up state
    StringList messages = {0};
    Arena* arena = ArenaMake();
    
    u64 counts[MG_Gen_COUNT] = {0};
    StringList hLists[MG_Gen_COUNT] = {0};
    StringList cLists[MG_Gen_COUNT] = {0};
    
    //- long: search and parse metadesk files
    MG_NodeList parses = {0};
    String projectDir = StrChopLastSlash(OSGetExeDir());
    String codeDir    = StrPushf(arena, "%.*s\\code", StrExpand(projectDir));
    String ignoreDir  = StrPushf(arena, "%.*s\\retired", StrExpand(codeDir));
    
    DeferBlock(Outf("parsing metadesk..."), Outf(" %lld metadesk file(s) parsed\n", parses.count))
    {
        FileIterBlock(arena, it, codeDir, FileIterFlag_SkipFolders|FileIterFlag_Recursive)
        {
            if (StrCompare(it.path, ignoreDir, 0))
                continue;
            
            String filepath = StrPushf(arena, "%.*s/%.*s", StrExpand(it.path), StrExpand(it.name));
            
            if (StrCompare(StrSkipLastDot(filepath), StrLit("mdesk"), 0))
            {
                String data = OSReadFile(arena, filepath);
                MD_ParseResult parse = MD_ParseText(arena, filepath, data);
                
                for (MD_Msg* msg = parse.msgs.first; msg; msg = msg->next)
                {
                    char* level = 0;
                    switch (msg->kind)
                    {
                        default: break;
                        case MD_MsgKind_Note:        level = "note";        break;
                        case MD_MsgKind_Warning:     level = "warning";     break;
                        case MD_MsgKind_Error:       level = "error";       break;
                        case MD_MsgKind_FatalError:  level = "fatal error"; break;
                    }
                    
                    TextLoc location = TextLocFromOff(data, msg->node->src_offset);
                    StrListPushf(arena, &messages, "%s:%u:%u: %s: %.*s\n", filepath.str,
                                 location.line, location.col, level, StrExpand(msg->string));
                }
                
                MG_Node* parseNode = PushStruct(arena, MG_Node);
                SLLQueuePush(parses.first, parses.last, parseNode);
                parseNode->node = parse.root;
                parses.count += 1;
            }
        }
    }
    
    //- long: gather tables
    MG_Map tableMap = MG_PushMap(arena, 1024);
    u64 tableCount = 0;
    DeferBlock(Outf("gathering tables..."), Outf(" %llu table(s) found\n", tableCount))
    {
        for (MG_Node* file = parses.first; file; file = file->next)
        {
            for (MD_EachNode(node, file->node->first))
            {
                MD_Node* tag = md_tag_from_string(node, StrLit("table"), 0);
                if (!md_node_is_nil(tag))
                {
                    MG_Table* table = MG_PushTable(arena, node, tag);
                    MG_MapInsert(arena, tableMap, node->string, table);
                    tableCount++;
                }
            }
        }
    }
    
    //- long: generate meta code
    Outf("generating meta code... ");
    for (MG_Node* file = parses.first; file != 0; file = file->next)
    {
        for (MD_EachNode(node, file->node->first))
        {
            String name = node->string;
            StringList genList = MG_StrListFromTable(arena, tableMap, node, StrLit("expand"));
            
            for (MD_EachNode(tag, node->first_tag))
            {
                String tagArg = tag->first->string;
                StringList* lists = md_node_has_tag(node, StrLit("c_file"), 0) ? cLists : hLists;
                MG_GenType type = 0;
                StringList* out = 0;
                
                local const char* genFmt[MG_Gen_COUNT] =
                {
                    0,
                    0, "%.*s;\n", "%.*s;\n", // enum, union, and struct
                    0,         0, "%.*s,\n", // text, embed, and table
                    0,             "%.*s\n", // function and catch-all
                };
                
                local const String genFooter[MG_Gen_COUNT] =
                {
                    StrConst(""),
                    StrConst("};\n\n"), StrConst("};\n\n"), StrConst("};\n\n"), // enum, union, and struct
                    StrConst(      ""), StrConst(      ""), StrConst("};\n\n"), // text, embed, and table
                    StrConst(""),                           StrConst(    "\n"), // function and catch-all
                };
                
                local const String mg_tagNames[] =
                {
                    StrConst(""),
                    StrConst("enum"), StrConst("union"), StrConst("struct"),
                    StrConst("text"), StrConst("embed"), StrConst("data"),
                    StrConst(    ""), StrConst("gen"),
                };
                
                local const String mg_genArg[] =
                {
                    StrConst(""),
                    StrConst("enums"), StrConst("unions"), StrConst("structs"),
                    StrConst(     ""), StrConst(      ""), StrConst("tables"),
                    StrConst("functions"), StrConst(""),
                };
                
                for (; type < MG_Gen_COUNT; ++type)
                {
                    if (StrCompare(mg_tagNames[type], tag->string, 0))
                    {
                        MG_GenType target = type;
                        out = lists + type;
                        
                        if (type == MG_Gen_All && tagArg.size)
                        {
                            for (MG_GenType subtype = 0; subtype < MG_Gen_COUNT; ++subtype)
                            {
                                if (StrCompare(mg_genArg[subtype], tagArg, 0))
                                {
                                    out = lists + subtype;
                                    target = subtype;
                                    break;
                                }
                            }
                        }
                        
                        counts[target]++;
                        break;
                    }
                }
                
                if (!out) continue;
                
#define StrExpand3(str) StrExpand(str), StrExpand(str), StrExpand(str)
#define CGlobalStr "readonly global "
#define CTypedefStr(base) "typedef " base "%.*s %.*s;\n"
#define StrListPushTypedef(base) StrListPushf(arena, out, CTypedefStr(base " ") base " %.*s\n{\n", StrExpand3(name))
                
                switch (type)
                {
                    case MG_Gen_Enum:
                    {
                        String prefix = name;
                        if (StrCompare(StrPostfix(name, 5), StrLit("Flags"), 0))
                            prefix = StrChop(name, 1);
                        
                        if (tagArg.size == 0)
                            StrListPushTypedef("enum");
                        else
                            StrListPushf(arena, out, CTypedefStr("") "enum\n{\n", StrExpand(tagArg), StrExpand(name));
                        
                        for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                        {
                            String escaped = StrCEscape(arena, genNode->string);
                            StrListPushf(arena, out, "%.*s_%.*s,\n", StrExpand(prefix), StrExpand(escaped));
                        }
                    } break;
                    
                    case MG_Gen_Union:  StrListPushTypedef( "union"); break;
                    case MG_Gen_Struct: StrListPushTypedef("struct"); break;
                    case MG_Gen_Table:  StrListPushf(arena, out, CGlobalStr "%.*s %.*s[] =\n{\n",
                                                     StrExpand(tagArg), StrExpand(name)); break;
                    
                    case MG_Gen_Text:
                    {
                        String embed = MG_StrCFromMultiLine(arena, node->first->string);
                        StrListPushf(arena, out, "readonly global String %.*s = StrConst\n(%.*s);\n\n",
                                     StrExpand(name), StrExpand(embed));
                    } break;
                    
                    case MG_Gen_Embed:
                    {
                        String data = OSReadFile(arena, node->first->string);
                        String embed = MG_ArrCFromData(arena, data);
                        StrListPushf(arena, out,
                                     CGlobalStr "u8 %.*s__data[] =\n{\n%.*s};\n\n"
                                     CGlobalStr "String %.*s = {%.*s__data, sizeof(%.*s__data)};\n\n",
                                     StrExpand(name), StrExpand(embed), StrExpand3(name));
                    } break;
                    
                    case MG_Gen_Function: PANIC("Not Implemented"); break;
                    case MG_Gen_All: break;
                    default: PANIC("Unreachable"); break;
                }
                
                if (genFmt[type])
                {
                    for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                    {
                        String trimmed = StrTrimWspace(genNode->string);
                        String escaped = StrCEscape(arena, trimmed);
                        StrListPushf(arena, out, genFmt[type], StrExpand(escaped));
                    }
                }
                
                if (genFooter[type].size)
                    StrListPush(arena, out, genFooter[type]);
                
#undef StrExpand3
#undef CGlobalStr
#undef CTypedefStr
#undef StrListPushTypedef
            }
        }
    }
    
    Outf("[%llu types, %llu arrays, %llu generators] generated\n",
         counts[MG_Gen_Enum] + counts[MG_Gen_Union] + counts[MG_Gen_Struct],
         counts[MG_Gen_Text] + counts[MG_Gen_Embed] + counts[MG_Gen_Table],
         counts[MG_Gen_Function] + counts[MG_Gen_All]);
    
    //- long: write code to files
    b32 success = 0;
    DeferBlock(Outf("writting to files..."), Outf(success ? " done\n" : " failed\n"))
    {
        String generatedFolder = StrPushf(arena, "%.*s\\%s", StrExpand(codeDir), "generated");
        if (OSCreateDir(generatedFolder))
        {
            String hPath = StrPushf(arena, "%.*s/test.meta.h", StrExpand(generatedFolder));
            String cPath = StrPushf(arena, "%.*s/test.meta.c", StrExpand(generatedFolder));
            
            String hData = {0}, cData = {0};
            ScratchBlock(scratch, arena)
            {
                hData = StrJoinListArr(arena, hLists, ArrayCount(hLists), 0, &(StringJoin){
                                           .pre = StrLit("//- GENERATED H CODE\n\n"
                                                         "#ifndef TEST_META_H\n#"
                                                         "define TEST_META_H\n\n"),
                                           .post = StrLit("#endif // TEST_META_H\n")
                                       });
                
                cData = StrJoinListArr(arena, cLists, ArrayCount(cLists), 0,
                                       &(StringJoin){ .pre = "//- GENERATED C CODE\n\n" });
            }
            
            success = 1;
            if (hData.size) success &= OSWriteFile(hPath, hData);
            if (cData.size) success &= OSWriteFile(cPath, cData);
        }
    }
    
    //- long: print all messages to stderr
    StrListIter(&messages, node)
        Errf("%s", node->string.str);
    
    return 0;
}
