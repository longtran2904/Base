#include "Base.h"
#include "Base.c"

#include "LongMD.h"
#include "Metagen.h"
#include "Metagen.c"

i32 main(i32 argc, char** argv)
{
    OSSetArgs(argc, argv);
    
    //////////////////////////////
    //- rjf: set up state
    //
    //MG_MsgList msgs = {0};
    Arena* arena = ArenaMake();
    MG_State* state = PushStruct(arena, MG_State);
    
    //////////////////////////////
    //- rjf: search code directories for all files to consider
    //
    StringList file_paths = {0};
    String project_dir_path = StrChopLastSlash(OSGetExeDir());
    String code_dir_path    = StrPushf(arena, "%.*s/src", StrExpand(project_dir_path));
    DeferBlock(Outf("searching %.*s...", StrExpand(code_dir_path)), Outf(" %i files found\n", (i32)file_paths.nodeCount))
    {
        FileIterBlock(arena, it, code_dir_path, FileIterFlag_SkipFolders|FileIterFlag_Recursive)
        {
            String file_path = StrPushf(arena, "%.*s/%.*s", StrExpand(it.path), StrExpand(it.name));
            StrListPush(arena, &file_paths, file_path);
        }
    }
    
    //////////////////////////////
    //- rjf: parse all metadesk files
    //
    MG_NodeList parses = {0};
    DeferBlock(Outf("parsing metadesk..."), Outf(" %lld metadesk files parsed\n", parses.count))
    {
        for (StringNode* n = file_paths.first; n != 0; n = n->next)
        {
            String filepath = n->string;
            if (StrCompare(StrSkipLastDot(filepath), StrLit("mdesk"), 0))
            {
                String data = OSReadFile(arena, filepath, 0);
                MD_TokenizeResult tokenize = md_tokenize_from_text(arena, data);
                MD_ParseResult parse = md_parse_from_text_tokens(arena, filepath, data, tokenize.tokens);
                for (MD_Msg* m = parse.msgs.first; m != 0; m = m->next)
                {
                    String msg_kind_string = {0};
                    switch (m->kind)
                    {
                        default:{}break;
                        case MD_MsgKind_Note:        {msg_kind_string = StrLit("note");}break;
                        case MD_MsgKind_Warning:     {msg_kind_string = StrLit("warning");}break;
                        case MD_MsgKind_Error:       {msg_kind_string = StrLit("error");}break;
                        case MD_MsgKind_FatalError:  {msg_kind_string = StrLit("fatal error");}break;
                    }
                    /*TxtPt pt = mg_txt_pt_from_string_off(data, m->node->src_offset);
                    String location = StrPushf(arena, "%.*s:%I64d:%I64d", StrExpand(filepath), pt.line, pt.column);
                    MG_Msg dst_m = {location, msg_kind_string, m->string};
                    mg_msg_list_push(arena, &msgs, &dst_m);*/
                }
                MG_Node* parseNode = PushStruct(arena, MG_Node);
                SLLQueuePush(parses.first, parses.last, parseNode);
                parseNode->node = parse.root;
                parses.count += 1;
            }
        }
    }
    
    //////////////////////////////
    //- rjf: gather tables
    //
    MG_Map table_grid_map = MG_PushMap(arena, 1024);
    MG_Map table_col_map = MG_PushMap(arena, 1024);
    u64 table_count = 0;
    DeferBlock(Outf("gathering tables..."), Outf(" %i tables found\n", (i32)table_count))
    {
        for (MG_Node* n = parses.first; n != 0; n = n->next)
        {
            MD_Node* file = n->node;
            for (MD_EachNode(node, file->first))
            {
                MD_Node* table_tag = md_tag_from_string(node, StrLit("table"), 0);
                if (!md_node_is_nil(table_tag))
                {
                    MG_NodeGrid* table = PushStruct(arena, MG_NodeGrid);
                    StringList* col_descs = PushStruct(arena, StringList);
                    *table = MG_GridFromNode(arena, node);
                    *col_descs = MG_ColDescFromTag(arena, table_tag);
                    MG_MapInsert(arena, &table_grid_map, node->string, table);
                    MG_MapInsert(arena, &table_col_map, node->string, col_descs);
                    table_count += 1;
                }
            }
        }
    }
    
    //////////////////////////////
    //- rjf: generate enums
    //
    for (MG_Node* n = parses.first; n != 0; n = n->next)
    {
        MD_Node* file = n->node;
        for (MD_EachNode(node, file->first))
        {
            MD_Node* tag = md_tag_from_string(node, StrLit("enum"), 0);
            if (!md_node_is_nil(tag))
            {
                String enum_name = node->string;
                String enum_member_prefix = enum_name;
                if (StrCompare(StrPostfix(enum_name, 5), StrLit("Flags"), 0))
                {
                    enum_member_prefix = StrChop(enum_name, 1);
                }
                String enum_base_type_name = tag->first->string;
                StringList gen_strings = MG_StrListFromTable(arena, table_grid_map, table_col_map, node);
                if (enum_base_type_name.size == 0)
                {
                    StrListPushf(arena, &state->enums, "typedef enum %.*s\n{\n", StrExpand(enum_name));
                }
                else
                {
                    StrListPushf(arena, &state->enums, "typedef %.*s %.*s;\n", StrExpand(enum_base_type_name), StrExpand(enum_name));
                    StrListPushf(arena, &state->enums, "typedef enum %.*sEnum\n{\n", StrExpand(enum_name));
                }
                for (StringNode* genStr = gen_strings.first; genStr != 0; genStr = genStr->next)
                {
                    String escaped = MG_StrCEscape(arena, genStr->string);
                    StrListPushf(arena, &state->enums, "%.*s_%.*s,\n", StrExpand(enum_member_prefix), StrExpand(escaped));
                }
                if (enum_base_type_name.size == 0)
                {
                    StrListPushf(arena, &state->enums, "} %.*s;\n\n", StrExpand(enum_name));
                }
                else
                {
                    StrListPushf(arena, &state->enums, "} %.*sEnum;\n\n", StrExpand(enum_name));
                }
            }
        }
    }
    
    //////////////////////////////
    //- rjf: generate structs
    //
    for (MG_Node* n = parses.first; n != 0; n = n->next)
    {
        MD_Node* file = n->node;
        for (MD_EachNode(node, file->first))
        {
            if (md_node_has_tag(node, StrLit("struct"), 0))
            {
                StringList gen_strings = MG_StrListFromTable(arena, table_grid_map, table_col_map, node);
                StrListPushf(arena, &state->structs, "typedef struct %.*s %.*s;\n", StrExpand(node->string), StrExpand(node->string));
                StrListPushf(arena, &state->structs, "struct %.*s\n{\n", StrExpand(node->string));
                for (StringNode* genStr = gen_strings.first; genStr != 0; genStr = genStr->next)
                {
                    String escaped = MG_StrCEscape(arena, genStr->string);
                    StrListPushf(arena, &state->structs, "%.*s;\n", StrExpand(escaped));
                }
                StrListPushf(arena, &state->structs, "};\n\n");
            }
        }
    }
    
    //////////////////////////////
    //- rjf: generate data tables
    //
    for (MG_Node* n = parses.first; n != 0; n = n->next)
    {
        MD_Node* file = n->node;
        for (MD_EachNode(node, file->first))
        {
            MD_Node* tag = md_tag_from_string(node, StrLit("data"), 0);
            if (!md_node_is_nil(tag))
            {
                String element_type = tag->first->string;
                StringList gen_strings = MG_StrListFromTable(arena, table_grid_map, table_col_map, node);
                if (!md_node_has_tag(node, StrLit("c_file"), 0))
                {
                    StrListPushf(arena, &state->h_tables, "extern %.*s %.*s[%I64u];\n",
                                 StrExpand(element_type), StrExpand(node->string), gen_strings.nodeCount);
                }
                StrListPushf(arena, &state->c_tables, "%.*s %.*s[%I64u] =\n{\n",
                             StrExpand(element_type), StrExpand(node->string), gen_strings.nodeCount);
                for (StringNode* genStr = gen_strings.first; genStr != 0; genStr = genStr->next)
                {
                    String escaped = MG_StrCEscape(arena, genStr->string);
                    StrListPushf(arena, &state->c_tables, "%.*s,\n", StrExpand(escaped));
                }
                StrListPush(arena, &state->c_tables, StrLit("};\n\n"));
            }
        }
    }
    
    //////////////////////////////
    //- rjf: generate enum -> string mapping functions
    //
    for (MG_Node* n = parses.first; n != 0; n = n->next)
    {
        MD_Node* file = n->node;
        for (MD_EachNode(node, file->first))
        {
            MD_Node* tag = md_tag_from_string(node, StrLit("enum2string_switch"), 0);
            if (!md_node_is_nil(tag))
            {
                String enum_type = tag->first->string;
                StringList gen_strings = MG_StrListFromTable(arena, table_grid_map, table_col_map, node);
                StrListPushf(arena, &state->h_functions, "internal String %.*s(%.*s v);\n", StrExpand(node->string), StrExpand(enum_type));
                StrListPushf(arena, &state->c_functions, "internal String\n%.*s(%.*s v)\n{\n", StrExpand(node->string), StrExpand(enum_type));
                StrListPushf(arena, &state->c_functions, "String result = StrLit(\"<Unknown %.*s>\");\n", StrExpand(enum_type));
                StrListPushf(arena, &state->c_functions, "switch (v)\n");
                StrListPushf(arena, &state->c_functions, "{\n");
                StrListPushf(arena, &state->c_functions, "default:{}break;\n");
                for (StringNode* genStr = gen_strings.first; genStr != 0; genStr = genStr->next)
                {
                    String escaped = MG_StrCEscape(arena, genStr->string);
                    StrListPushf(arena, &state->c_functions, "%.*s;\n", StrExpand(escaped));
                }
                StrListPushf(arena, &state->c_functions, "}\n");
                StrListPushf(arena, &state->c_functions, "return result;\n");
                StrListPushf(arena, &state->c_functions, "}\n\n");
            }
        }
    }
    
    //////////////////////////////
    //- rjf: gather & generate all embeds
    //
    for (MG_Node* n = parses.first; n != 0; n = n->next)
    {
        MD_Node* file = n->node;
        for (MD_EachNode(node, file->first))
        {
            if (md_node_has_tag(node, StrLit("embed_string"), 0))
            {
                String embed_string = MG_StrCFromMultiLine(arena, node->first->string);
                StrListPushf(arena, &state->h_tables, "read_only global String %.*s =\nStrLit_comp(\n", StrExpand(node->string));
                StrListPush (arena, &state->h_tables, embed_string);
                StrListPushf(arena, &state->h_tables, ");\n\n");
            }
            if (md_node_has_tag(node, StrLit("embed_file"), 0))
            {
                String data = OSReadFile(arena, node->first->string, 1);
                String embed_string = MG_ArrCFromData(arena, data);
                StrListPushf(arena, &state->h_tables, "read_only global U8 %.*s__data[] =\n{\n", StrExpand(node->string));
                StrListPush (arena, &state->h_tables, embed_string);
                StrListPushf(arena, &state->h_tables, "};\n\n");
                StrListPushf(arena, &state->h_tables, "read_only global String %.*s = {%.*s__data, sizeof(%.*s__data)};\n",
                             StrExpand(node->string),
                             StrExpand(node->string),
                             StrExpand(node->string));
            }
        }
    }
    
    return 0;
}
