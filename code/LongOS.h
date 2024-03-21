/* date = July 26th 2022 4:17 pm */

#ifndef _LONG_O_S_H
#define _LONG_O_S_H

//~ NOTE(long): Setup

function void InitOSMain(int argc, char **argv);
function StringList GetOSArgs(void);

//~ NOTE(long): Exit

function void ExitOSProcess(u32 code);

//~ NOTE(long): Base Memory Functions

function void* OSReserve(u64 size);
function void  OSRelease(void* ptr);

function b32  OSCommit  (void* ptr, u64 size);
function void OSDecommit(void* ptr, u64 size);

//~ NOTE(long): File Handling

function String ReadOSFile(Arena* arena, String fileName, b32 terminateData);
function b32    WriteOSList(String fileName, StringList* data);
#define WriteOSFile(file, data) WriteOSList((file), &(StringList) \
                                            { \
                                                .first = &(StringNode){ .string = (data) }, \
                                                .last  = &(StringNode){ .string = (data) }, \
                                                .nodeCount = 1, .totalSize = (data).size, \
                                            })

function FileProperties GetFileProperties(String fileName);
function String GetCurrDir(Arena* arena);
function String GetProcDir(void);
function String GetUserDir(void);
function String GetTempDir(void);

function b32 DeleteOSFile(String fileName);
function b32 RenameOSFile(String oldName, String newName);
function b32 MakeOSDir(String path);
function b32 DeleteOSDir(String path);

//~ NOTE(long): File Iteration

typedef struct OSFileIter
{
    u8 v[640];
} OSFileIter;

function OSFileIter InitFileIter(String path);
function b32 NextFileIter(Arena* arena, OSFileIter* iter, String* outName, FileProperties* outProp);
function void EndFileIter(OSFileIter* iter);

//~ NOTE(long): Time

function void     SleepMilliseconds(u32 t);
function u64      NowMicroseconds(void);
function DateTime NowUniversalTime(void);
function DateTime ToLocalTime(DateTime* universalTime);
function DateTime ToUniversalTime(DateTime* localTime);

//~ NOTE(long): Libraries

typedef struct OSLib OSLib;
struct OSLib
{
    u64 v[1];
};

function OSLib     LoadOSLib(String path);
function VoidFunc* GetOSProc(OSLib lib, char* name);
function void      ReleaseOSLib(OSLib lib);

//~ NOTE(long): Entropy

function void GetOSEntropy(void* data, u64 size);

#endif //_LONG_O_S_H
