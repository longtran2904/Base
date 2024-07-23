#include "Base.h"
#include "Base.c"

#include "LongMD.h"

i32 main(i32 argc, char** argv)
{
    OSInit(argc, argv);
#if 0
    Arena* arena = ArenaMake();
    StringList files = {0};
    //- TODO(long): Setup paths and search for files
    
    //- NOTE(long): Parse all metadesk files
    MD_Node* list = MD_MakeList(arena);
    i32 count = 0;
    DeferBlock(Outf("parsing metadesk..."), Outf(" %i metadesk files parsed\n", count))
    {
        for (StringNode *n = files.first; n != 0; n = n->next)
        {
            String path = n->string;
            if (StrCompare(StrSkipUntil(path, StrLit("."), FindStr_LastMatch), StrLit("mdesk"), 0))
            {
                MD_ParseResult parse = MD_ParseWholeFile(arena, MD_STR(path));
                for (MD_Message* message = parse.errors.first; message != 0; message = message->next)
                    MD_PrintMessage(stdout, MD_CodeLocFromNode(message->node), message->kind, message->string);
                MD_PushNewReference(arena, list, parse.node);
                count++;
            }
        }
    }
    
    //////////////////////////////
    //- rjf: gather tables
    //
    MG_Map table_grid_map = mg_push_map(mg_arena, 1024);
    MG_Map table_col_map = mg_push_map(mg_arena, 1024);
    i32 table_count = 0;
    DeferBlock(Outf("gathering tables..."), Outf(" %i tables found\n", table_count))
    {
        for (MD_EachNode(root_it, list->first_child))
        {
            MD_Node* root = MD_ResolveNodeFromReference(root_it);
            for (MD_EachNode(node, root->first_child))
            {
                MD_Node* table_tag = MD_TagFromString(node, str8_lit("table"), 0);
                if(!md_node_is_nil(table_tag))
                {
                    MG_NodeGrid *table = push_array(mg_arena, MG_NodeGrid, 1);
                    MG_ColumnDescArray *col_descs = push_array(mg_arena, MG_ColumnDescArray, 1);
                    *table = mg_node_grid_make_from_node(mg_arena, node);
                    *col_descs = mg_column_desc_array_from_tag(mg_arena, table_tag);
                    mg_map_insert_ptr(mg_arena, &table_grid_map, node->string, table);
                    mg_map_insert_ptr(mg_arena, &table_col_map, node->string, col_descs);
                    table_count++;
                }
            }
        }
    }
#endif
    
    return 0;
}
