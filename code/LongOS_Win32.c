// NOTE(long): 28301: annotations weren't found at the first declaration of a given function
// I have zero idea what this warning does but it keep reports something stupid in winnt.h
// winnt.h(3454) : warning C28301: No annotations for first declaration of '_mm_clflush'. See <no file>(0). 
MSVC(WarnDisable(28301))
#undef function
#include <Windows.h>
#include <Userenv.h>
#include <dwmapi.h>
#define function static
MSVC(WarnEnable(28301))

/* NOTE(long): This all the win32 APIs that I need
VirtualAlloc
VirtualFree
MEM_XXX, PAGE_XXX

BYTE
CHAR
DWORD
HINSTANCE
HANDLE
INVALID_HANDLE_VALUE
CloseHandle

LARGE_INTEGER
QueryPerformanceFrequency

GetLastError
ERROR_XXX

GetModuleFileNameA
GetCurrentProcessToken
GetUserProfileDirectoryA
GetTempPathA
GetCurrentDirectoryA

SYSTEMTIME
FILETIME
Sleep
GetSystemTime
SystemTimeToFileTime
FileTimeToLocalFileTime
FileTimeToSystemTime
LocalFileTimeToFileTime

FILE_ATTRIBUTE_XXX
WIN32_FILE_ATTRIBUTE_DATA
GetFileAttributesEx

GENERIC_XXX
XXX_ALWAYS
CreateFileA
DeleteFile
ReadFile
WriteFile
MoveFile
CreateDirectory
RemoveDirectory

WIN32_FIND_DATAA
FindFirstFile
FindNextFile
FindClose

HMODULE
LoadLibraryA
FreeLibrary

HCRYPTPROV
PROV_DSS, CRYPT_VERIFYCONTEXT
CryptAcquireContextA
CryptGenRandom
CryptReleaseContext
*/

//~ NOTE(long): Base Memory Functions

function void* OSReserve(u64 size)
{
    // NOTE(long): Afaik, PAGE_XXX only matters when you MEM_COMMIT the memory
    // so when you MEM_RESERVE the memory, you can pass in any PAGE_XXX as you want
    // as long as it's valid (non-zero and a valid PAGE_XXX).
    void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    if (!result)
    {
        DEBUG(error, DWORD error = GetLastError());
        PANIC("Failed to reserve memory");
    }
    return result;
}

function b32 OSCommit(void* ptr, u64 size)
{
    b32 result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0;
    if (!result)
    {
        DEBUG(error, DWORD error = GetLastError());
        PANIC("Failed to commit memory");
    }
    return result;
}

// NOTE(long): 6250: calling 'VirtualFree' without the MEM_RELEASE flag might cause address space leaks
MSVC(WarnDisable(6250))
function void OSDecommit(void* ptr, u64 size)
{
    if (!VirtualFree(ptr, size, MEM_DECOMMIT))
    {
        DEBUG(error, DWORD error = GetLastError());
        PANIC("Failed to decommit memory");
    }
}
MSVC(WarnEnable(6250))

function void OSRelease(void* ptr)
{
    if (!VirtualFree(ptr, 0, MEM_RELEASE))
    {
        DEBUG(error, DWORD error = GetLastError());
        PANIC("Failed to release memory");
    }
}

//~ NOTE(long): Global Variables

global u64 win32TicksPerSecond = 0;
global StringList win32CmdLine = {0};
global HINSTANCE win32Instance = {0};

global Arena* win32PermArena = {0};
global String win32BinaryPath = {0};
global String win32UserPath = {0};
global String win32TempPath = {0};

//~ NOTE(long): Setup

function void OSInit(int argc, char **argv)
{
    //- Setup precision time
    {
        LARGE_INTEGER perfFreq = {0};
        if (QueryPerformanceFrequency(&perfFreq))
            win32TicksPerSecond = ((u64)perfFreq.HighPart << 32) | perfFreq.LowPart;
        timeBeginPeriod(1);
    }
    
    //- Setup arena/args
    {
        win32PermArena = ArenaMake();
        
        for (int i = 0; i <argc; ++i)
        {
            String arg = StrFromCStr(argv[i]);
            StrListPush(win32PermArena, &win32CmdLine, arg);
        }
    }
    
    ScratchBegin(scratch);
    
    //- Setup binary path
    {
        DWORD cap = 2048;
        u8* buffer = 0;
        DWORD size = 0;
        for (u64 r = 0; r < 4; ++r, cap *= 4)
        {
            u8* tryBuffer = PushArrayNZ(scratch, u8, cap);
            DWORD trySize = GetModuleFileNameA(0, tryBuffer, cap);
            if (trySize == cap && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                ScratchClear(scratch);
            else
            {
                buffer = tryBuffer;
                size = trySize;
                break;
            }
        }
        
        if (ALWAYS(size))
            win32BinaryPath = StrCopy(win32PermArena, StrChopAfter(Str(buffer, size), StrLit("/\\"), FindStr_LastMatch));
    }
    
    //- Setup user path
    {
        HANDLE token = GetCurrentProcessToken();
        DWORD cap = 2048;
        u8* buffer = PushArrayNZ(scratch, u8, cap);
        if (!GetUserProfileDirectoryA(token, buffer, &cap))
        {
            ScratchClear(scratch);
            buffer = PushArrayNZ(scratch, u8, cap);
            if (!GetUserProfileDirectoryA(token, buffer, &cap))
                buffer = 0;
        }
        
        if (ALWAYS(buffer))
            // NOTE(long): the docs make it sound like we can only count on cap getting the size on failure
            // so we're just going to cstring this to be safe.
            win32UserPath = StrCloneCStr(win32PermArena, buffer);
    }
    
    //- Setup temp path
    {
        DWORD cap = 2048;
        u8* buffer = PushArrayNZ(scratch, u8, cap);
        DWORD size = GetTempPathA(cap, buffer);
        if (size >= cap)
        {
            ScratchClear(scratch);
            buffer = PushArrayNZ(scratch, u8, size);
            size = GetTempPathA(size, buffer);
        }
        
        // NOTE(long): size - 1 because this particular string function in the Win32 API
        // is different from the others and it includes the trailing backslash.
        // We want consistency, so the "- 1" removes it.
        if (ALWAYS(size > 1))
            win32TempPath = StrCopy(win32PermArena, Str(buffer, size - 1));
    }
    
    ScratchEnd(scratch);
}

function StringList OSCmdArgs(void)
{
    return win32CmdLine;
}

//~ NOTE(long): Exit

function void OSExitProcess(u32 code)
{
    ExitProcess(code);
}

//~ NOTE(long): Win32 Specialized Functions

function void W32WinMainInit(HINSTANCE hInstance,
                             HINSTANCE hPrevInstance,
                             LPSTR lpCmdLine,
                             int nShowCmd)
{
    win32Instance = hInstance;
    UNUSED(hPrevInstance);
    UNUSED(lpCmdLine);
    UNUSED(nShowCmd);
    
    OSInit(__argc, __argv);
}

function HINSTANCE W32GetInstance(void)
{
    return win32Instance;
}

#define GET_PROC_ADDR(f, m, n) (*(PROC*)(&(f))) = GetProcAddress((m), (n))

//~ NOTE(long): Time

function DateTime W32DateTimeFromSystemTime(SYSTEMTIME* time)
{
    DateTime result = {0};
    result.year = (i16)time->wYear;
    result.mon  =  (u8)time->wMonth;
    result.day  =  (u8)time->wDay;
    result.hour =  (u8)time->wHour;
    result.min  =  (u8)time->wMinute;
    result.sec  =  (u8)time->wSecond;
    result.msec = (u16)time->wMilliseconds;
    return result;
}

function SYSTEMTIME W32SystemTimeFromDateTime(DateTime time)
{
    SYSTEMTIME result = {0};
    result.wYear = time.year;
    result.wMonth = time.mon;
    result.wDay = time.day;
    result.wHour = time.hour;
    result.wMinute = time.min;
    result.wSecond = time.sec;
    result.wMilliseconds = time.msec;
    return result;
}

function DenseTime W32DenseTimeFromFileTime(FILETIME* fileTime)
{
    SYSTEMTIME systemTime = {0};
    FileTimeToSystemTime(fileTime, &systemTime);
    DateTime dateTime = W32DateTimeFromSystemTime(&systemTime);
    DenseTime result = TimeToDense(dateTime);
    return result;
}

function void OSSleepMS(u32 ms)
{
    Sleep(ms);
}

function u64 OSNowMS(void)
{
    u64 result = 0;
    LARGE_INTEGER perfCounter = {0};
    if (QueryPerformanceCounter(&perfCounter))
    {
        u64 ticks = ((u64)perfCounter.HighPart << 32) | perfCounter.LowPart;
        result = ticks * Thousand(1) / win32TicksPerSecond;
    }
    return result;
}

function DateTime OSNowUniTime(void)
{
    SYSTEMTIME systemTime = {0};
    GetSystemTime(&systemTime);
    DateTime result = W32DateTimeFromSystemTime(&systemTime);
    return result;
}

function DateTime OSToLocTime(DateTime universalTime)
{
    SYSTEMTIME universalSystemTime = W32SystemTimeFromDateTime(universalTime);
    FILETIME universalFileTime = {0};
    SystemTimeToFileTime(&universalSystemTime, &universalFileTime);
    FILETIME localFileTime = {0};
    FileTimeToLocalFileTime(&universalFileTime, &localFileTime);
    SYSTEMTIME localSystemTime = {0};
    FileTimeToSystemTime(&localFileTime, &localSystemTime);
    DateTime result = W32DateTimeFromSystemTime(&localSystemTime);
    return result;
}

function DateTime OSToUniTime(DateTime localTime)
{
    SYSTEMTIME localSystemTime = W32SystemTimeFromDateTime(localTime);
    FILETIME localFileTime = {0};
    SystemTimeToFileTime(&localSystemTime, &localFileTime);
    FILETIME universalFileTime = {0};
    LocalFileTimeToFileTime(&localFileTime, &universalFileTime);
    SYSTEMTIME universalSystemTime = {0};
    FileTimeToSystemTime(&universalFileTime, &universalSystemTime);
    DateTime result = W32DateTimeFromSystemTime(&universalSystemTime);
    return result;
}

//~ NOTE(long): Console Handling

internal String W32ReadFile(Arena* arena, HANDLE file, b32 terminateData)
{
    String result = {0};
    u64 size = 0;
    
    if (GetFileSizeEx(file, (PLARGE_INTEGER)&size) && size)
    {
        TempArena restorePoint = TempBegin(arena);
        u8* buffer = PushArray(arena, u8, size + !!terminateData);
        
        u8* ptr = buffer;
        u8* opl = buffer + size;
        b32 success = 1;
        for (DWORD actualRead = 0; ptr < opl && success; ptr += actualRead)
        {
            DWORD readAmount = (u32)ClampTop((u64)(opl - ptr), MAX_U32);
            // ReadFile will always zero out actualRead
            success =  ReadFile(file, ptr, readAmount, &actualRead, 0);
            success &= actualRead;
        }
        
        if (success)
            result = Str(buffer, size);
        else
            TempEnd(restorePoint);
    }
    
    return result;
}

// NOTE(long): CRT seems to be using 4KB for fgets internal buffer size
// But `cmd.exe` allows up to 8KB - 1 characters, so I use that instead
// https://learn.microsoft.com/en-us/troubleshoot/windows-client/shell-experience/command-line-string-limitation
#define CONSOLE_INTERNAL_BUFFER_SIZE KB(8)

function String OSReadConsole(Arena* arena, i32 handle, b32 terminateData)
{
    String result = {0};
    
    DWORD handles[] =
    {
        0,
        STD_INPUT_HANDLE,
        STD_OUTPUT_HANDLE,
        STD_ERROR_HANDLE,
    };
    if (NEVER(!InRange(handle, 0, 3)))
        handle = 0;
    
    HANDLE file = GetStdHandle(handles[handle]);
    if (file != INVALID_HANDLE_VALUE)
    {
        DWORD fileType = GetFileType(file);
        if (fileType == FILE_TYPE_DISK)
            result = W32ReadFile(arena, file, terminateData);
        
        else
        {
            TempArena restorePoint = TempBegin(arena);
            DWORD bufferSize = CONSOLE_INTERNAL_BUFFER_SIZE;
            DWORD actualRead = 0;
            DWORD readSize = bufferSize - !!terminateData;
            u8* buffer = PushArray(arena, u8, bufferSize);
            
            if (fileType == FILE_TYPE_PIPE)
            {
                // TODO(long): https://handmade.network/forums/t/8916-how_to_read_from_and_write_to_the_console_in_win32#30197
            }
            
            if (ReadFile(file, buffer, readSize, &actualRead, 0))
            {
                if (actualRead)
                {
                    result = Str(buffer, actualRead);
                    
                    // NOTE(long): Stupid Microsoft with stupid \r\n
                    if (StrCompare(StrPostfix(result, 2), StrLit("\r\n"), 0))
                        result.size -= 2;
                    else
                        Assert(actualRead == readSize);
                    
                    ArenaPop(arena, readSize - result.size);
                }
            }
            
            if (!result.size)
                TempEnd(restorePoint);
        }
    }
    
    return result;
}

function b32 OSWriteConsole(i32 handle, String data)
{
    b32 result = 0;
    
    DWORD handles[] =
    {
        0,
        STD_INPUT_HANDLE,
        STD_OUTPUT_HANDLE,
        STD_ERROR_HANDLE,
    };
    if (NEVER(!InRange(handle, 0, 3)))
        handle = 0;
    
    HANDLE file = GetStdHandle(handles[handle]);
    if (file != INVALID_HANDLE_VALUE)
    {
        StaticAssert(sizeof(DWORD) == sizeof(i32));
        DWORD writeAmount = ALWAYS(data.size <= MAX_I32) ? (DWORD)data.size : MAX_I32;
        DWORD actualWrite = 0;
        result = WriteFile(file, data.str, writeAmount, &actualWrite, 0);
        result = result && ALWAYS(data.size == actualWrite);
    }
    
    return result;
}

//~ NOTE(long): File Handling

function String OSReadFile(Arena* arena, String fileName, b32 terminateData)
{
    String result = {0};
    HANDLE file = CreateFileA(fileName.str,
                              GENERIC_READ, FILE_SHARE_READ, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file != INVALID_HANDLE_VALUE)
    {
        result = W32ReadFile(arena, file, terminateData);
        CloseHandle(file);
    }
    
    return result;
}

function b32 OSWriteList(String fileName, StringList* data)
{
    b32 result = 0;
    HANDLE file = CreateFileA(fileName.str,
                              GENERIC_WRITE, FILE_SHARE_WRITE, 0,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file != INVALID_HANDLE_VALUE)
    {
        result = 1;
        
        StrListIter(data, node)
        {
            u8* ptr = node->string.str;
            u8* opl = ptr + node->string.size;
            
            for (; ptr < opl;)
            {
                DWORD writeAmount = (u32)ClampTop((u64)(opl - ptr), MAX_U32);
                DWORD actualWrite = 0;
                result = WriteFile(file, ptr, writeAmount, &actualWrite, 0);
                if (result)
                    goto END;
                
                ptr += actualWrite;
            }
        }
        
        END:
        CloseHandle(file);
    }
    
    return result;
}

function FilePropertyFlags W32FilePropertyFlagsFromAttributes(DWORD attributes)
{
    FilePropertyFlags result = 0;
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        result |= FilePropertyFlag_IsFolder;
    return result;
}

function DataAccessFlags W32AccessFromAttributes(DWORD attributes)
{
    DataAccessFlags result = 0;
    if (attributes & FILE_ATTRIBUTE_READONLY)
        result = DataAccessFlag_Read|DataAccessFlag_Execute;
    else
        result = DataAccessFlag_Read|DataAccessFlag_Write|DataAccessFlag_Execute;
    return result;
}

function FileProperties OSFileProperties(String fileName)
{
    FileProperties result = {0};
    WIN32_FILE_ATTRIBUTE_DATA attributes = {0};
    if (GetFileAttributesEx(fileName.str, GetFileExInfoStandard, &attributes))
    {
        result = (FileProperties)
        {
            .size = ((u64)attributes.nFileSizeHigh << 32)|(u64)attributes.nFileSizeLow,
            .flags = W32FilePropertyFlagsFromAttributes(attributes.dwFileAttributes),
            .createTime = W32DenseTimeFromFileTime(&attributes.ftCreationTime),
            .modifyTime = W32DenseTimeFromFileTime(&attributes.ftLastWriteTime),
            .access = W32AccessFromAttributes(attributes.dwFileAttributes),
        };
    }
    
    return result;
}

function String OSExecDir(void) { return win32BinaryPath; }
function String OSUserDir(void) { return win32UserPath; }
function String OSTempDir(void) { return win32TempPath; }
function String OSCurrDir(Arena* arena)
{
    DWORD size = GetCurrentDirectoryA(0, 0);
    u8* buffer = PushArrayNZ(arena, u8, size);
    size = GetCurrentDirectoryA(size + 1, buffer);
    return Str(buffer, size);
}

function b32 OSDeleteFile(String fileName)
{
    b32 result = DeleteFile(fileName.str);
    return result;
}

function b32 OSRenameFile(String oldName, String newName)
{
    b32 result = MoveFile(oldName.str, newName.str);
    return result;
}

function b32 OSCreateDir(String path)
{
    b32 result = CreateDirectory(path.str, 0);
    return result;
}

function b32 OSDeleteDir(String path)
{
    b32 result = RemoveDirectory(path.str);
    return result;
}

// NOTE(long): File Iteration

typedef struct W32FileIter
{
    HANDLE handle;
    WIN32_FIND_DATAA findData; // WIN32_FIND_DATAA is 320 bytes while WIN32_FIND_DATAW is 592 bytes
    b32 done;
} W32FileIter;

StaticAssert(ArrayCount(((OSFileIter*)0)->v) >= sizeof(W32FileIter), w32fileiter);

function OSFileIter FileIterInit(String path)
{
    OSFileIter result = {0};
    ScratchBlock(scratch)
    {
        path = StrJoin3(scratch, path, StrLit("\\*"));
        W32FileIter* w32Iter = (W32FileIter*)result.v;
        w32Iter->handle = FindFirstFile(path.str, &w32Iter->findData);
    }
    return result;
}

function b32 FileIterNext(Arena* arena, OSFileIter* iter)
{
    b32 result = false;
    
    W32FileIter* w32Iter = (W32FileIter*)iter->v;
    if (w32Iter->handle != 0 && w32Iter->handle != INVALID_HANDLE_VALUE)
    {
        while (!w32Iter->done)
        {
            // Check for . and ..
            CHAR* fileName = w32Iter->findData.cFileName;
            b32 isDot = (fileName[0] == '.' && fileName[1] == 0);
            b32 isDotDot = (fileName[0] == '.' && fileName[1] == '.' && fileName[2] == 0);
            
            b32 emit = !isDot && !isDotDot;
            WIN32_FIND_DATAA data = {0};
            if (emit)
                data = w32Iter->findData;//CopyStruct(&data, &w32Iter->findData);
            
            // Increment the iter
            if (!FindNextFile(w32Iter->handle, &w32Iter->findData))
                w32Iter->done = true;
            
            // Do the emit if we saved one earlier
            if (emit)
            {
                iter->name = StrCloneCStr(arena, data.cFileName);
                iter->prop = (FileProperties)
                {
                    .size = ((u64)data.nFileSizeHigh << 32) | (u64)data.nFileSizeLow,
                    .flags = W32FilePropertyFlagsFromAttributes(data.dwFileAttributes),
                    .access = W32AccessFromAttributes(data.dwFileAttributes),
                    .createTime = W32DenseTimeFromFileTime(&data.ftCreationTime),
                    .modifyTime = W32DenseTimeFromFileTime(&data.ftLastWriteTime),
                };
                
                result = true;
                break;
            }
        }
    }
    
    return result;
}

function void FileIterEnd(OSFileIter* iter)
{
    W32FileIter* w32Iter = (W32FileIter*)iter->v;
    if (w32Iter->handle != 0 && w32Iter->handle != INVALID_HANDLE_VALUE)
        FindClose(w32Iter->handle);
}

//~ NOTE(long): Libraries

function OSLib OSLoadLib(String path)
{
    OSLib result = {0};
    result.v[0] = (u64)LoadLibraryA(path.str);
    return result;
}

function VoidFunc* OSGetProc(OSLib lib, char* name)
{
    HMODULE module = (HMODULE)lib.v[0];
    VoidFunc* result = (VoidFunc*)(GetProcAddress(module, name));
    return result;
}

function void OSFreeLib(OSLib lib)
{
    HMODULE module = (HMODULE)lib.v[0];
    FreeLibrary(module);
}

//~ NOTE(long): Entropy

function void OSGetEntropy(void* data, u64 size)
{
    HCRYPTPROV prov = 0;
    CryptAcquireContextA(&prov, 0, 0, PROV_DSS, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(prov, (u32)ClampTop(size, MAX_U32), (BYTE*)data);
    CryptReleaseContext(prov, 0);
}
