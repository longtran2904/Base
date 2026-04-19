#include "Base.h"
#include "Base.c"
#include "parser/LongScanner.h"
#include "parser/LongScanner.c"
#include "xlsxio_read.h"
#include "xlsxio_write.h"

#define SHEET_PREFIX "=== SHEET: "
#define SHEET_POSTFIX " ===\n"

String StrFromExcel(Arena* arena, char* path, char sep)
{
    String result = {0};
    
    xlsxioreader excelReader = xlsxioread_open(path);
    if (excelReader == NULL)
    {
        Errf("Error opening .xlsx file\n");
        return result;
    }
    
    xlsxioreadersheetlist sheetList = xlsxioread_sheetlist_open(excelReader);
    if (sheetList == NULL)
    {
        Errf("Error opening sheet list\n");
        return result;
    }
    
    ScratchBegin(scratch, arena);
    StringList list = {0};
    const char* sheetName;
    
    while ((sheetName = xlsxioread_sheetlist_next(sheetList)) != NULL)
    {
        xlsxioreadersheet sheet = xlsxioread_sheet_open(excelReader, sheetName, XLSXIOREAD_SKIP_EMPTY_ROWS);
        if (sheet == NULL)
            continue;
        
        StrListPushf(scratch, &list, SHEET_PREFIX "%s" SHEET_POSTFIX, sheetName);
        
        while (xlsxioread_sheet_next_row(sheet))
        {
            char* value;
            while ((value = xlsxioread_sheet_next_cell(sheet)) != NULL)
            {
                StrListPushf(scratch, &list, "%s%c", value, sep);
                xlsxioread_free(value);
            }
            
            StrListPushf(scratch, &list, "\n");
        }
        
        StrListPushf(scratch, &list, "\n");
        xlsxioread_sheet_close(sheet);
    }
    
    result = StrJoin(arena, &list);
    result = StrTrimEnd(result, "\n");
    ScratchEnd(scratch);
    
    xlsxioread_sheetlist_close(sheetList);
    xlsxioread_close(excelReader);
    return result;
}

b32 ExcelWrite(String path, String text)
{
    ScratchBegin(scratch);
    StringList sheets = StrSplit(scratch, text, StrLit(SHEET_PREFIX), 0);
    
    for (StringNode* node = sheets.first; node; node = node->next)
    {
        StringJoin sheet = SubstrSplit(node->string, StrLit(SHEET_POSTFIX));
        String name = sheet.pre;
        String data = sheet.post;
        
        // TODO(long): Replace this library with one that can actually writes multiple sheets
        xlsxiowriter handle = xlsxiowrite_open(path.str, StrToCStr(scratch, name));
        if (handle == NULL)
        {
            Errf("Error creating .xlsx file\n");
            return 0;
        }
        
        StringTable table = StrTableFromStr(scratch, data, '\t', '\n');
        String prefix = {0};
        for (i32 row = 0; row < table.rowCount; ++row)
        {
            StringList cells = table.rows[row];
            
            for (StringNode* col = cells.first; col; col = col->next)
            {
                if (prefix.size)
                {
                    col->string = StrPushf(scratch, "%.*s: %.*s", StrExpand(prefix), StrExpand(col->string));
                    prefix = ZeroStr;
                }
                
                else if (StrStartsWith(col->string, StrLit("<<<<<<<"), 0))
                {
                    prefix = StrLit("OURS");
                    goto SKIP_ROW;
                }
                
                else if (StrStartsWith(col->string, StrLit("======="), 0))
                {
                    prefix = StrLit("THEIRS");
                    goto SKIP_ROW;
                }
                
                else if (StrStartsWith(col->string, StrLit(">>>>>>>"), 0))
                {
                    goto SKIP_ROW;
                }
                
                char* str = StrToCStr(scratch, col->string);
                if (row == 0) //write column names
                    xlsxiowrite_add_column(handle, str, 0);
                else
                    xlsxiowrite_add_cell_string(handle, str);
            }
            
            xlsxiowrite_next_row(handle);
            SKIP_ROW:;
        }
        
        xlsxiowrite_close(handle);
    }
    
    ScratchEnd(scratch);
    return 1;
}

i32 main(i32 argc, char **argv)
{
    ScratchBegin(scratch);
    b32 result = 0;
    char sep = '\t';
    
    if (argc == 2)
    {
        String tsv = StrFromExcel(scratch, argv[1], sep);
        if ((result = !!tsv.size))
            Outf("%.*s", StrExpand(tsv));
    }
    
    else if (argc >= 4)
    {
        StringList args = OSSetArgs(argc, argv);
        String files[3] = {0};
        i32 count = 0;
        
        String oursName = {0};
        String oursData = {0};
        
        for (StringNode* node = args.first->next; node && count < ArrayCount(files) && !result; node = node->next, ++count)
        {
            String data = StrFromExcel(scratch, node->string.str, sep);
            files[count] = StrPushf(scratch, "%.*s.temp_tsv", StrExpand(node->string));
            
            if (count == 1)
            {
                oursName = node->string;
                oursData = data;
            }
            
            if (!OS_PathWrite(files[count], data))
            {
                Errf("Error creating .temp_tsv files\n");
                result = 2;
            }
        }
        
        if (!result)
        {
            // NOTE(long): git merge driver is `base ours theirs` (%O %A %B)
            // while git merge-file is `ours base theirs`
            String cmd = StrPushf(scratch, "git merge-file %.*s %.*s %.*s",
                                  StrExpand(files[1]), StrExpand(files[0]), StrExpand(files[2]));
            
            if (OS_ProcessExec(cmd, &(OS_ProcessParams){0}))
            {
                String diff = OS_PathRead(scratch, files[1]);
                if (!StrCompare(diff, oursData, 0))
                {
                    if (StrCompare(args.last->string, StrLit("-debug"), 0))
                        oursName = StrPushf(scratch, "%.*s.debug.xlsx", StrExpand(oursName));
                    b32 noWrite = StrCompare(args.last->string, StrLit("-nowrite"), 0);
                    if (noWrite || ExcelWrite(oursName, diff))
                        result = 1;
                    else
                        result = 2;
                }
            }
        }
        
        OS_FileDelete(files[0]);
        OS_FileDelete(files[1]);
        OS_FileDelete(files[2]);
    }
    
    ScratchEnd(scratch);
    return result;
}
