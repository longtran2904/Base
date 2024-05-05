/* date = July 26th 2022 4:17 pm */

#ifndef _LONG_O_S_H
#define _LONG_O_S_H

//~ NOTE(long): Setup

function void OSInit(int argc, char **argv);
function StringList OSCmdArgs(void);

//~ NOTE(long): Exit

function void OSExitProcess(u32 code);

//~ NOTE(long): Base Memory Functions

function void* OSReserve(u64 size);
function void  OSRelease(void* ptr);

function b32    OSCommit(void* ptr, u64 size);
function void OSDecommit(void* ptr, u64 size);

//~ NOTE(long): File Handling

function String OSReadFile(Arena* arena, String fileName, b32 terminateData);
function b32    OSWriteList(String fileName, StringList* data);
#define OSWriteFile(file, data) OSWriteList((file), &(StringList) \
                                            { \
                                                .first = &(StringNode){ .string = (data) }, \
                                                .last  = &(StringNode){ .string = (data) }, \
                                                .nodeCount = 1, .totalSize = (data).size, \
                                            })

function b32 OSDeleteFile(String fileName);
function b32 OSRenameFile(String oldName, String newName);
function b32 OSCreateDir(String path);
function b32 OSDeleteDir(String path);

function FileProperties GetFileProperties(String fileName);

//~ NOTE(long): File Iteration

typedef struct OSFileIter
{
    u8 v[640];
} OSFileIter;

function OSFileIter FileIterInit(String path);
function b32 FileIterNext(Arena* arena, OSFileIter* iter, String* outName, FileProperties* outProp);
function void FileIterEnd(OSFileIter* iter);

//~ NOTE(long): Paths

function String OSCurrentDir(Arena* arena);
function String OSProcessDir(void);
function String OSAppDataDir(void);
function String OSAppTempDir(void);

//~ NOTE(long): Time

function void OSSleepMS(u32 ms);
function u64  OSNowMS(void);

function DateTime OSNowUniTime(void);
function DateTime OSToLocTime(DateTime* universalTime);
function DateTime OSToUniTime(DateTime* localTime);

//~ NOTE(long): Libraries

typedef struct OSLib OSLib;
struct OSLib
{
    u64 v[1];
};

function OSLib     OSLoadLib(String path);
function void      OSFreeLib(OSLib lib);
function VoidFunc* OSGetProc(OSLib lib, char* name);

//~ NOTE(long): Entropy

function void OSGetEntropy(void* data, u64 size);

#endif //_LONG_O_S_H
