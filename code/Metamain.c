#include "Base.h"
#include "Base.c"

#include "LongMD.h"
#include "Metagen.h"
#include "Metagen.c"

i32 main(i32 argc, char** argv)
{
    OSSetArgs(argc, argv);
    
    //////////////////////////////
    //- long: set up state
    //
    StringList messages = {0};
    Arena* arena = ArenaMake();
    //MG_State* state = PushStruct(arena, MG_State);
    
    /*StringList enums = {0}, unions = {0}, structs = {0};
    StringList h_functions = {0}, h_tables = {0}, h_catchall = {0};
    StringList c_functions = {0}, c_tables = {0}, c_catchall = {0};*/
    
#define MG_ZeroList &(StringList){0}
    
    StringList* hLists[MG_Gen_COUNT] = { MG_ZeroList,
        MG_ZeroList, MG_ZeroList, MG_ZeroList,
        MG_ZeroList, MG_ZeroList, MG_ZeroList,
        MG_ZeroList, MG_ZeroList, };
    StringList* cLists[MG_Gen_COUNT] = { MG_ZeroList,
        MG_ZeroList, MG_ZeroList, MG_ZeroList,
        MG_ZeroList, MG_ZeroList, MG_ZeroList,
        MG_ZeroList, MG_ZeroList, };
    u64 counts[MG_Gen_COUNT] = {0};
    
    //////////////////////////////
    //- long: search and parse metadesk files
    //
    MG_NodeList parses = {0};
    String projectDir = StrChopLastSlash(OSGetExeDir());
    String codeDir    = StrPushf(arena, "%.*s\\code", StrExpand(projectDir));
    
    DeferBlock(Outf("parsing metadesk..."), Outf(" %lld metadesk file(s) parsed\n", parses.count))
    {
        FileIterBlock(arena, it, codeDir, FileIterFlag_SkipFolders|FileIterFlag_Recursive)
        {
            String filepath = StrPushf(arena, "%.*s/%.*s", StrExpand(it.path), StrExpand(it.name));
            
            if (StrCompare(StrSkipLastDot(filepath), StrLit("mdesk"), 0))
            {
                String data = OSReadFile(arena, filepath, 0);
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
                    StrListPushf(arena, &messages, "%s%u:%u: %s: %.*s\n", filepath.str,
                                 location.line, location.col, level, StrExpand(msg->string));
                }
                
                MG_Node* parseNode = PushStruct(arena, MG_Node);
                SLLQueuePush(parses.first, parses.last, parseNode);
                parseNode->node = parse.root;
                parses.count += 1;
            }
        }
    }
    
    //////////////////////////////
    //- long: gather tables
    //
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
                StringList** lists = md_node_has_tag(node, StrLit("c_file"), 0) ? cLists : hLists;
                MG_GenType type = 0;
                StringList* out = 0;
                
                for (; type < MG_Gen_COUNT; ++type)
                {
                    if (StrCompare(mg_tagNames[type], tag->string, 0))
                    {
                        MG_GenType target = type;
                        out = lists[type];
                        
                        if (type == MG_Gen_All && tagArg.size)
                        {
                            for (MG_GenType subtype = 0; subtype < MG_Gen_COUNT; ++subtype)
                            {
                                if (StrCompare(mg_genArg[subtype], tagArg, 0))
                                {
                                    out = lists[subtype];
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
                
#define MG_StrExpand3(str) StrExpand(str), StrExpand(str), StrExpand(str)
#define MG_StrListPushTypedef(arena, list, base, name) \
    StrListPushf(arena, list, "typedef " base " %.*s %.*s\n" base " %.*s\n{\n", MG_StrExpand3(name))
                
                switch (type)
                {
                    default: break;
                    
                    case MG_Gen_Enum:
                    {
                        String prefix = name;
                        if (StrCompare(StrPostfix(name, 5), StrLit("Flags"), 0))
                            prefix = StrChop(name, 1);
                        
                        if (tagArg.size == 0)
                            MG_StrListPushTypedef(arena, out, "enum", name);
                        else
                            StrListPushf(arena, out, "typedef %.*s %.*s;\nenum\n{\n",
                                         StrExpand(tagArg), StrExpand(name));
                        
                        for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                        {
                            String escaped = StrCEscape(arena, genNode->string);
                            StrListPushf(arena, out, "%.*s_%.*s,\n", StrExpand(prefix), StrExpand(escaped));
                        }
                        StrListPush(arena, out, StrLit("};\n\n"));
                    } break;
                    
                    case MG_Gen_Union:
                    {
                        MG_StrListPushTypedef(arena, out, "union", name);
                        for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                        {
                            String escaped = StrCEscape(arena, genNode->string);
                            StrListPushf(arena, out, "%.*s;\n", StrExpand(escaped));
                        }
                        StrListPush(arena, out, StrLit("};\n\n"));
                    } break;
                    
                    case MG_Gen_Struct:
                    {
                        MG_StrListPushTypedef(arena, out, "struct", name);
                        for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                        {
                            String escaped = StrCEscape(arena, genNode->string);
                            StrListPushf(arena, out, "%.*s;\n", StrExpand(escaped));
                        }
                        StrListPush(arena, out, StrLit("};\n\n"));
                    } break;
                    
                    case MG_Gen_Table:
                    {
                        StrListPushf(arena, out, "%.*s %.*s[%I64u] =\n{\n",
                                     StrExpand(tagArg), StrExpand(node->string), genList.nodeCount);
                        
                        for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                        {
                            String escaped = StrCEscape(arena, genNode->string);
                            StrListPushf(arena, out, "%.*s,\n", StrExpand(escaped));
                        }
                        
                        StrListPush(arena, out, StrLit("};\n\n"));
                    } break;
                    
                    case MG_Gen_Text:
                    {
                        String embed = MG_StrCFromMultiLine(arena, node->first->string);
                        StrListPushf(arena, out, "readonly global String %.*s = StrConst\n(%.*s);\n\n",
                                     StrExpand(name), StrExpand(embed));
                    } break;
                    
                    case MG_Gen_Embed:
                    {
                        String data = OSReadFile(arena, node->first->string, 1);
                        String embed = MG_ArrCFromData(arena, data);
                        StrListPushf(arena, out,
                                     "readonly global u8 %.*s__data[] =\n{\n%.*s};\n\n"
                                     "readonly global String %.*s = {%.*s__data, sizeof(%.*s__data)};\n",
                                     StrExpand(name), StrExpand(embed), MG_StrExpand3(name));
                    } break;
                    
                    case MG_Gen_Function:
                    {
                        PANIC("Not Implemented");
                    } break;
                    
                    case MG_Gen_All:
                    {
                        for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                        {
                            String trimmed = StrTrimWspace(genNode->string);
                            String escaped = StrCEscape(arena, trimmed);
                            StrListPushf(arena, out, "%.*s\n", StrExpand(escaped));
                        }
                        
                        StrListPush(arena, out, StrLit("\n"));
                    } break;
                }
                
#if 0
                //- long: generate enums
                if (StrCompare(tag->string, StrLit("enum"), 0))
                {
                    String prefix = name;
                    if (StrCompare(StrPostfix(name, 5), StrLit("Flags"), 0))
                        prefix = StrChop(name, 1);
                    
                    String baseType = tag->first->string;
                    if (baseType.size == 0)
                        StrListPushf(arena, &state->enums, "typedef enum %.*s %.*s\nenum %.*s\n{\n",
                                     StrExpand(name), StrExpand(name), StrExpand(name));
                    else
                        StrListPushf(arena, &state->enums, "typedef %.*s %.*s;\nenum\n{\n",
                                     StrExpand(baseType), StrExpand(name));
                    
                    for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                    {
                        String escaped = StrCEscape(arena, genNode->string);
                        StrListPushf(arena, footer, "%.*s_%.*s,\n", StrExpand(prefix), StrExpand(escaped));
                    }
                    
                    StrListPush(arena, footer, StrLit("};\n\n"));
                }
                
                //- long: generate structs
                else if (StrCompare(tag->string, StrLit("struct"), 0))
                {
                    footer = &state->structs;
                    StrListPushf(arena, &state->structs, "typedef struct %.*s %.*s;\nstruct %.*s\n{\n",
                                 StrExpand(node->string), StrExpand(node->string), StrExpand(node->string));
                    
                    for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                    {
                        String escaped = StrCEscape(arena, genNode->string);
                        StrListPushf(arena, footer, "%.*s;\n", StrExpand(escaped));
                    }
                    
                    StrListPush(arena, footer, StrLit("};\n\n"));
                }
                
                //- long: generate unions
                else if (StrCompare(tag->string, StrLit("union"), 0))
                {
                    footer = &state->structs;
                    StrListPushf(arena, &state->structs, "typedef union %.*s %.*s;\nunion %.*s\n{\n",
                                 StrExpand(node->string), StrExpand(node->string), StrExpand(node->string));
                    
                    for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                    {
                        String escaped = StrCEscape(arena, genNode->string);
                        StrListPushf(arena, footer, "%.*s;\n", StrExpand(escaped));
                    }
                    
                    StrListPush(arena, footer, StrLit("};\n\n"));
                }
                
                //- long: generate data tables
                else if (StrCompare(tag->string, StrLit("data"), 0))
                {
                    footer = &state->c_tables;
                    String baseType = tag->first->string;
                    if (!md_node_has_tag(node, StrLit("c_file"), 0))
                        StrListPushf(arena, &state->h_tables, "extern %.*s %.*s[%I64u];\n",
                                     StrExpand(baseType), StrExpand(node->string), genList.nodeCount);
                    
                    StrListPushf(arena, &state->c_tables, "%.*s %.*s[%I64u] =\n{\n",
                                 StrExpand(baseType), StrExpand(node->string), genList.nodeCount);
                    
                    for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                    {
                        String escaped = StrCEscape(arena, genNode->string);
                        StrListPushf(arena, footer, "%.*s,\n", StrExpand(escaped));
                    }
                    
                    StrListPush(arena, footer, StrLit("};\n\n"));
                }
                
                //- long: generate catch-all generations
                else if (StrCompare(tag->string, StrLit("gen"), 0))
                {
                    b32 fileC = md_node_has_tag(node, StrLit("c_file"), 0);
                    StringList* out = fileC ? &state->c_catchall : &state->h_catchall;
                    
                    if (tag->first->string.size == 0) { }
                    else if (StrCompare(tag->first->string, StrLit(    "enums"), 0)) out = &state->enums;
                    else if (StrCompare(tag->first->string, StrLit(  "structs"), 0)) out = &state->structs;
                    else if (StrCompare(tag->first->string, StrLit(   "tables"), 0)) out = fileC ? &state->   c_tables : &state->   h_tables;
                    else if (StrCompare(tag->first->string, StrLit("functions"), 0)) out = fileC ? &state->c_functions : &state->h_functions;
                    
                    for (StringNode* genNode = genList.first; genNode != 0; genNode = genNode->next)
                    {
                        String trimmed = StrTrimWspace(genNode->string);
                        String escaped = StrCEscape(arena, trimmed);
                        StrListPushf(arena, out, "%.*s\n", StrExpand(escaped));
                    }
                    
                    StrListPush(arena, out, StrLit("\n\n"));
                }
                
                //- long: generate text embeds
                else if (md_node_has_tag(node, StrLit("text"), 0))
                {
                    String embed = MG_StrCFromMultiLine(arena, node->first->string);
                    StrListPushf(arena, &state->h_tables, "readonly global String %.*s = StrConst\n(%.*s);\n\n",
                                 StrExpand(node->string), StrExpand(embed));
                }
                
                //- long: generate file embeds
                else if (md_node_has_tag(node, StrLit("embed"), 0))
                {
                    String data = OSReadFile(arena, node->first->string, 1);
                    String embed = MG_ArrCFromData(arena, data);
                    StrListPushf(arena, &state->h_tables,
                                 "readonly global u8 %.*s__data[] =\n{\n%.*s};\n\n"
                                 "readonly global String %.*s = {%.*s__data, sizeof(%.*s__data)};\n",
                                 StrExpand(node->string), StrExpand(embed),
                                 StrExpand(node->string), StrExpand(node->string), StrExpand(node->string));
                }
#endif
            }
        }
    }
    /*Outf("[%llu enums, %llu structs, %llu tables] generated\n",
    state->enums.nodeCount, state->structs.nodeCount, state->h_tables.nodeCount + state->c_tables.nodeCount);*/
    Outf("[%llu enums, %llu types, %llu globals, %llu functions, %llu generators] generated\n",
         counts[MG_Gen_Enum], counts[MG_Gen_Union] + counts[MG_Gen_Struct],
         counts[MG_Gen_Text] + counts[MG_Gen_Embed] + counts[MG_Gen_Table],
         counts[MG_Gen_Function] + counts[MG_Gen_Text]);
    
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
                hData = StrListJoin(arena, hLists, ArrayCount(hLists),
                                    .pre = StrLit("//- GENERATED H CODE\n\n"
                                                  "#ifndef TEST_META_H\n#"
                                                  "define TEST_META_H\n\n"),
                                    .post = StrLit("#endif // TEST_META_H\n"));
                
                cData = StrListJoin(arena, cLists, ArrayCount(cLists), .pre = "//- GENERATED C CODE\n\n");
            }
            
            if (hData.size)
                success  = OSWriteFile(hPath, hData);
            
            if (cData.size)
                success &= OSWriteFile(cPath, cData);
        }
    }
    
    //- long: print all messages to stderr
    StrListIter(&messages, node)
        Errf("%s", node->string.str);
    
    return 0;
}
