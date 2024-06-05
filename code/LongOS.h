/* date = July 26th 2022 4:17 pm */

#ifndef _LONG_O_S_H
#define _LONG_O_S_H

//~ NOTE(long): Base Setup

#if !defined(MemReserve)
#define MemReserve(size) OSReserve(size)
#endif
#if !defined(MemCommit)
#define MemCommit(ptr, size) OSCommit(ptr, size)
#endif
#if !defined(MemDecommit)
#define MemDecommit(ptr, size) OSDecommit((ptr), (size))
#endif
#if !defined(MemRelease)
#define MemRelease(ptr) OSRelease(ptr)
#endif

#ifndef PrintOut
#define PrintOut(str) OSWriteConsole(OS_STD_OUT, (str))
#endif
#ifndef PrintErr
#define PrintErr(str) OSWriteConsole(OS_STD_ERR, (str))
#endif

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

//~ NOTE(long): Console Handling

enum
{
    OS_STD_NONE,
    OS_STD_IN,
    OS_STD_OUT,
    OS_STD_ERR,
};

function String OSReadConsole(Arena* arena, i32 handle, b32 terminateData);
function b32   OSWriteConsole(i32 handle, String data);

//~ NOTE(long): File Handling

function String OSReadFile(Arena* arena, String fileName, b32 terminateData);
function b32   OSWriteList(String fileName, StringList* data);
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

function FileProperties OSFileProperties(String fileName);

//~ NOTE(long): File Iteration

typedef struct OSFileIter
{
    String name;
    FileProperties prop;
    u8 v[640];
} OSFileIter;

function OSFileIter FileIterInit(String path);
function b32 FileIterNext(Arena* arena, OSFileIter* iter);
function void FileIterEnd(OSFileIter* iter);
#define FileIterBlock(arena, name, path) for (OSFileIter name = FileIterInit(path); \
                                              FileIterNext(arena, &name) ? 1 : (FileIterEnd(&name), 0);)

//~ NOTE(long): Paths

function String OSCurrDir(Arena* arena);
function String OSExecDir(void);
function String OSUserDir(void);
function String OSTempDir(void);

// @CONSIDER(long): Maybe have a OSSetCurrDir and OSGetInitDir

//~ NOTE(long): Time

function void OSSleepMS(u32 ms);
function u64  OSNowMS(void);

function DateTime OSNowUniTime(void);
function DateTime OSToLocTime(DateTime universalTime);
function DateTime OSToUniTime(DateTime localTime);

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

//~ TODO(long): Processes/Threads

//~ TODO(long): Network/Sockets/IPC

#endif //_LONG_O_S_H
