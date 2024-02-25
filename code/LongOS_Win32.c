#undef function
#include <Windows.h>
#include <Userenv.h>
#define function static

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

//~ NOTE: Base Memory Functions

function void* OSReserve(u64 size)
{
    void * result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    return result;
}

function b32 OSCommit(void* ptr, u64 size)
{
    b32 result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0;
    return result;
}

function void OSDecommit(void* ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

function void OSRelease(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

//~ NOTE: Global Variables

//global DWORD win32OSThreadContextIndex = 0;
global DWORD win32TicksPerSecond = 0;

global String win32BinaryPath = {0};
global String win32UserPath = {0};
global String win32TempPath = {0};

global StringList win32CmdLine = {0};
global HINSTANCE win32Instance = {0};

//~ NOTE: Setup

function void InitOSMain(int argc, char **argv)
{
    // Setup precision time
    LARGE_INTEGER perfFreq = {0};
    if (QueryPerformanceFrequency(&perfFreq))
        win32TicksPerSecond = ((u64)perfFreq.HighPart << 32) | perfFreq.LowPart;
    timeBeginPeriod(1);
    
    BeginScratch(scratch);
    
    // Setup arena
    permArena = ArenaMake();
    
    for (int i = 0; i <argc; ++i)
    {
        String arg = StrFromCStr((u8*)argv[i]);
        StrListPush(permArena, &win32CmdLine, arg);
    }
    
    {
        DWORD cap = 2048;
        u8* buffer = 0;
        DWORD size = 0;
        for (u64 r = 0; r < 4; ++r, cap *= 4)
        {
            u8* tryBuffer = PushArray(scratch, u8, cap);
            DWORD trySize = GetModuleFileNameA(0, tryBuffer, cap);
            if (trySize == cap && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                ResetScratch(scratch);
            else
            {
                buffer = tryBuffer;
                size = trySize;
                break;
            }
        }
        
        String fullPath = Str(buffer, size);
        String binaryPath = StrPrefix(fullPath, StrFindChr(fullPath, "/\\", FindStr_LastMatch));
        win32BinaryPath = StrCopy(permArena, binaryPath);
    }
    
    {
        HANDLE token = GetCurrentProcessToken();
        DWORD cap = 2048;
        u8* buffer = PushArray(scratch, u8, cap);
        if (!GetUserProfileDirectoryA(token, buffer, &cap))
        {
            ResetScratch(scratch);
            buffer = PushArray(scratch, u8, cap);
            if (!GetUserProfileDirectoryA(token, buffer, &cap))
                buffer = 0;
        }
        
        if (buffer != 0)
            // NOTE: the docs make it sound like we can only count on cap getting the
            // size on failure; so we're just going to cstring this to be safe.
            win32UserPath = StrCloneCStr(permArena, buffer);
    }
    
    {
        DWORD cap = 2048;
        u8* buffer = PushArray(scratch, u8, cap);
        DWORD size = GetTempPathA(cap, buffer);
        if (size >= cap)
        {
            ResetScratch(scratch);
            buffer = PushArray(scratch, u8, size);
            size = GetTempPathA(size, buffer);
        }
        
        // NOTE: size - 1 because this particular string function in the Win32 API
        // is different from the others and it includes  the trailing backslash.
        // We want consistency, so the "- 1" removes it.
        win32TempPath = StrCopy(permArena, Str(buffer, size - 1));
    }
    
    EndScratch(scratch);
}

function StringList GetOSArgs(void)
{
    return win32CmdLine;
}

//~ NOTE: Exit

function void ExitOSProcess(u32 code)
{
    ExitProcess(code);
}

//~ NOTE: Win32 Specialized Functions

function void W32WinMainInit(HINSTANCE hInstance,
                             HINSTANCE hPrevInstance,
                             LPSTR lpCmdLine,
                             int nShowCmd)
{
    int argc = __argc;
    char** argv = __argv;
    win32Instance = hInstance;
    InitOSMain(argc, argv);
}

function HINSTANCE W32GetInstance(void)
{
    return win32Instance;
}

#define GET_PROC_ADDR(f, m, n) (*(PROC*)(&(f))) = GetProcAddress((m), (n))

//~ NOTE: Time

function DateTime W32DateTimeFromSystemTime(SYSTEMTIME* time)
{
    DateTime result = {0};
    result.year = time->wYear;
    result.mon = (u8)time->wMonth;
    result.day = time->wDay - 1;
    result.hour = time->wHour;
    result.min = time->wMinute;
    result.sec = time->wSecond;
    result.msec = time->wMilliseconds;
    return result;
}

function SYSTEMTIME W32SystemTimeFromDateTime(DateTime* time)
{
    SYSTEMTIME result = {0};
    result.wYear = time->year;
    result.wMonth = time->mon + 1;
    result.wDay = time->day + 1;
    result.wHour = time->hour;
    result.wMinute = time->min;
    result.wSecond = time->sec;
    result.wMilliseconds = time->msec;
    return result;
}

function DenseTime W32DenseTimeFromFileTime(FILETIME* fileTime)
{
    SYSTEMTIME systemTime = {0};
    FileTimeToSystemTime(fileTime, &systemTime);
    DateTime dateTime = W32DateTimeFromSystemTime(&systemTime);
    DenseTime result = TimeToDense(&dateTime);
    return result;
}

function void SleepOSMillisecond(u32 t)
{
    Sleep(t);
}

function u64 NowMicroseconds(void)
{
    u64 result = 0;
    LARGE_INTEGER perfCounter = {0};
    if (QueryPerformanceCounter(&perfCounter))
    {
        u64 ticks = ((u64)perfCounter.HighPart << 32) | perfCounter.LowPart;
        result = ticks * Million(1) / win32TicksPerSecond;
    }
    return result;
}

function DateTime NowUniversalTime(void)
{
    SYSTEMTIME systemTime = {0};
    GetSystemTime(&systemTime);
    DateTime result = W32DateTimeFromSystemTime(&systemTime);
    return result;
}

function DateTime ToLocalTime(DateTime* universalTime)
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

function DateTime ToUniversalTime(DateTime* localTime)
{
    SYSTEMTIME localSystemTime = W32SystemTimeFromDateTime(localTime);
    FILETIME localFileTime = {0};
    SystemTimeToFileTime(&localSystemTime, &localFileTime);
    FILETIME universalFileTime = {0};
    LocalFileTimeToFileTime(&localFileTime, &universalFileTime);
    SYSTEMTIME universalSystemTime = {0};
    FileTimeToSystemTIme(&universalFileTime, &universalSystemTime);
    DateTime result = W32DateTimeFromSystemTime(&universalSystemTime);
    return result;
}

//~ NOTE: File Handling

function String ReadOSFile(Arena* arena, String fileName, b32 terminateData)
{
    BeginScratch(scratch, arena);
    HANDLE file = CreateFileA(fileName.str,
                              GENERIC_READ, 0, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                              0);
    
    String result = {0};
    if (file != INVALID_HANDLE_VALUE)
    {
        DWORD highSize = 0;
        DWORD lowSize = GetFileSize(file, &highSize);
        u64 totalSize = (((u64)highSize << 32) | (u64)lowSize);
        
        TempArena restorePoint = TempBegin(arena);
        u8* buffer = PushZeroArray(arena, u8, totalSize + (terminateData ? 1 : 0));
        
        u8* ptr = buffer;
        u8* opl = buffer + totalSize;
        b32 success = true;
        for (; ptr < opl;)
        {
            u64 totalAmount = (u64)(opl - ptr);
            DWORD readAmount = ClampTop(totalAmount, MAX_U32);
            DWORD actualRead = 0;
            if (!ReadFile(file, ptr, readAmount, &actualRead, 0))
            {
                success = false;
                break;
            }
            ptr += actualRead;
        }
        
        if (success)
            result = (String){ buffer, totalSize };
        else
            TempEnd(restorePoint);
        
        CloseHandle(file);
    }
    
    EndScratch(scratch);
    return result;
}

function b32 WriteOSList(String fileName, StringList data)
{
    BeginScratch(scratch);
    HANDLE file = CreateFileA(fileName.str,
                              GENERIC_WRITE, 0, 0,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                              0);
    
    b32 result = false;
    if (file != INVALID_HANDLE_VALUE)
    {
        result = true;
        
        for (StringNode* node = data.first; node != 0; node = node->next)
        {
            u8* ptr = node->string.str;
            u8* opl = ptr + node->string.size;
            
            for (; ptr < opl;)
            {
                u64 totalAmount = (u64)(opl - ptr);
                DWORD writeAmount = ClampTop(totalAmount, MAX_U32);
                DWORD actualWrite = 0;
                if (!WriteFile(file, ptr, writeAmount, &actualWrite, 0))
                {
                    result = false;
                    goto END;
                }
                
                ptr += actualWrite;
            }
        }
        
        END:
        CloseHandle(file);
    }
    
    EndScratch(scratch);
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

function FileProperties GetFileProperties(String fileName)
{
    BeginScratch(scratch);
    
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

function String GetFilePath(Arena* arena, SystemPath path)
{
    String result = {0};
    switch (path)
    {
        case SystemPath_CurrentDirectory:
        {
            BeginScratch(scratch, arena);
            DWORD cap = 2048;
            u8* buffer = PushArray(scratch, u8, cap);
            DWORD size = GetCurrentDirectoryA(cap, buffer);
            if (size > cap)
            {
                ResetScratch(scratch);
                buffer = PushArray(scratch, u8, size);
                size = GetCurrentDirectoryA(size, buffer);
            }
            result = StrCopy(arena, Str(buffer, size));
            EndScratch(scratch);
        } break;
        
        // @RECONSIDER(long): Should I just put all these in a read-only section and return, rather than copy all the times.
        case SystemPath_Binary:   result = StrCopy(arena, win32BinaryPath); break;
        case SystemPath_UserData: result = StrCopy(arena, win32UserPath  ); break;
        case SystemPath_TempData: result = StrCopy(arena, win32TempPath  ); break;
    }
    
    return result;
}

function b32 DeleteOSFile(String fileName)
{
    b32 result = DeleteFile(fileName.str);
    return result;
}

function b32 RenameOSFile(String oldName, String newName)
{
    b32 result = MoveFile(oldName.str, newName.str);
    return result;
}

function b32 MakeOSDir(String path)
{
    b32 result = CreateDirectory(path.str, 0);
    return result;
}

function b32 DeleteOSDir(String path)
{
    b32 result = RemoveDirectory(path.str);
    return result;
}

// NOTE: File Iteration

typedef struct W32FileIter
{
    HANDLE handle;
    WIN32_FIND_DATAA findData; // WIN32_FIND_DATAA is 320 bytes while WIN32_FIND_DATAW is 592 bytes
    b32 done;
} W32FileIter;

StaticAssert(sizeof(W32FileIter) <= sizeof(OSFileIter), w32fileiter);

function OSFileIter InitFileIter(String path)
{
    BeginScratch(scratch);
    
    path = StrJoin3(scratch, path, StrLit("\\*"));
    OSFileIter result = {0};
    W32FileIter* w32Iter = (W32FileIter*)&result;
    w32Iter->handle = FindFirstFile(path.str, &w32Iter->findData);
    
    EndScratch(scratch);
    return result;
}

function b32 NextFileIter(Arena* arena, OSFileIter* iter, String* outName, FileProperties* outProp)
{
    b32 result = false;
    
    W32FileIter* w32Iter = (W32FileIter*)iter;
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
                String name = StrCloneCStr(arena, data.cFileName);
                FileProperties prop = (FileProperties)
                {
                    ((u64)data.nFileSizeHigh << 32) | (u64)data.nFileSizeLow,
                    W32FilePropertyFlagsFromAttributes(data.dwFileAttributes),
                    W32DenseTimeFromFileTime(&data.ftCreationTime),
                    W32DenseTimeFromFileTime(&data.ftLastWriteTime),
                    W32AccessFromAttributes(data.dwFileAttributes)
                };
                
                if (outName)
                    *outName = name;
                if (outProp)
                    *outProp = prop;
                
                result = true;
                break;
            }
        }
    }
    
    return result;
}

function void EndFileIter(OSFileIter* iter)
{
    W32FileIter* w32Iter = (W32FileIter*)iter;
    if (w32Iter->handle != 0 && w32Iter->handle != INVALID_HANDLE_VALUE)
        FindClose(w32Iter->handle);
}

//~ NOTE: Libraries

function OSLib LoadOSLib(String path)
{
    OSLib result = {0};
    result.v[0] = (u64)LoadLibraryA(path.str);
    return result;
}

function VoidFunc* GetOSProc(OSLib lib, char* name)
{
    HMODULE module = (HMODULE)lib.v[0];
    VoidFunc* result = (VoidFunc*)(GetProcAddress(module, name));
    return result;
}

function void ReleaseOSLib(OSLib lib)
{
    HMODULE module = (HMODULE)lib.v[0];
    FreeLibrary(module);
}

//~ NOTE: Entropy

function void GetOSEntropy(void* data, u64 size)
{
    HCRYPTPROV prov = 0;
    CryptAcquireContextA(&prov, 0, 0, PROV_DSS, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(prov, size, (BYTE*)data);
    CryptReleaseContext(prov, 0);
}
