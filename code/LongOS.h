/* date = July 26th 2022 4:17 pm */

#ifndef _LONG_O_S_H
#define _LONG_O_S_H

//~ NOTE: Setup

function void InitOSMain(int argc, char **argv);
function StringList GetOSArgs(void);

//~ NOTE: Exit

function void ExitOSProcess(u32 code);

//~ NOTE: Base Memory Functions

function void* OSReserve(u64 size);
function b32 OSCommit(void* ptr, u64 size);
function void OSDecommit(void* ptr, u64 size);
function void OSRelease(void* ptr);

//~ NOTE: File Handling

typedef enum SystemPath
{
    SystemPath_CurrentDirectory,
    SystemPath_Binary,
    SystemPath_UserData,
    SystemPath_TempData,
    SystemPath_Count
} SystemPath;

function String ReadOSFile(MemArena* arena, String fileName, b32 terminateData);
function b32    WriteOSFile(String fileName, String data);
function b32    WriteOSList(String fileName, StringList data);

function FileProperties GetFileProperties(String fileName);
function String GetFilePath(MemArena* arena, SystemPath path, b32 addNullTerminator);

function b32 DeleteOSFile(String fileName);
function b32 RenameOSFile(String oldName, String newName);
function b32 MakeOSDir(String path);
function b32 DeleteOSDir(String path);

//~ NOTE: File Iteration

typedef struct OSFileIter
{
    u8 v[640];
} OSFileIter;

function OSFileIter InitFileIter(String path);
function b32 NextFileIter(MemArena* arena, OSFileIter* iter, String* outName, FileProperties* outProp, b32 terminate);
function void EndFileIter(OSFileIter* iter);

//~ NOTE: Time

function void     SleepMilliseconds(u32 t);
function u64      NowMicroseconds(void);
function DateTime NowUniversalTime(void);
function DateTime ToLocalTime(DateTime* universalTime);
function DateTime ToUniversalTime(DateTime* localTime);

//~ NOTE: Libraries

typedef struct OSLib OSLib;
struct OSLib
{
    u64 v[1];
};

function OSLib     LoadOSLib(String path);
function VoidFunc* GetOSProc(OSLib lib, char* name);
function void      ReleaseOSLib(OSLib lib);

//~ NOTE: Entropy

function void GetOSEntropy(void* data, u64 size);

//~ NOTE: Generic OS function (Put this to a LongOS.c file when there are more functions)

function b32 WriteOSFile(String fileName, String data)
{
    StringNode node = {0};
    StringList list = {0};
    PushStrListExplicit(&list, data, &node);
    b32 result = WriteOSList(fileName, list);
    return result;
}

#endif //_LONG_O_S_H
