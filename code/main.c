#include "DefaultMemory.h"
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"

#define EvalPrint(x)    printf("%s: %s = %d\n", #x, Stringify(x), (int)(x))
#define EvalPrintLL(x)  printf("%s: %s = %lld\n", #x, Stringify(x), (x))
#define EvalPrintU(x)   printf("%s: %s = %u\n", #x, Stringify(x), (u32)(x))
#define EvalPrintULL(x) printf("%s: %s = %llu\n", #x, Stringify(x), (x))
#define EvalPrintF(x)   printf("%s: %s = %f\n", #x, Stringify(x), (x))
#define EvalPrintC(x)   printf("%s: %s = %c\n", #x, Stringify(x), (x))
#define EvalPrintS(x)   printf("%s: %s = %s\n", #x, Stringify(x), (x))
#define EvalPrintStr(x) printf("%s: %s = %.*s\n", #x, Stringify(x), StrExpand(x))
#define EvalPrintPtr(x) printf("%s: %s = %p\n", #x, Stringify(x), (x))
#define EvalPrintLine printf("\n");

#define CODE_PATH 0

#if CODE_PATH == 0
typedef struct Test
{
    int a;
    int b;
    int c;
    int d;
} Test;

int main(void)
{
    printf("cl = %d\n", COMPILER_CL);
    printf("clang = %d\n", COMPILER_CLANG);
    printf("gcc = %d\n", COMPILER_GCC);
    printf("widnows = %d\n", OS_WIN);
    printf("linux = %d\n", OS_LINUX);
    printf("mac = %d\n", OS_MAC);
    printf("x64 = %d\n", ARCH_X64);
    printf("x86 = %d\n", ARCH_X86);
    printf("arm = %d\n", ARCH_ARM);
    printf("arm64 = %d\n", ARCH_ARM64);
    
    int foo[100];
    for (int i = 0; i < ArrayCount(foo); ++i)
        foo[i] = i;
    EvalPrint(ArrayCount(foo));
    
    int bar[100];
    CopyArray(bar, foo);
    EvalPrint(bar[50]);
    EvalPrint(CompareMem(foo, bar, sizeof(foo)));
    
    EvalPrint(OffsetOf(Test, a));
    EvalPrint(OffsetOf(Test, b));
    EvalPrint(OffsetOf(Test, d));
    
    EvalPrintLine;
    
    Test t = { 1, 2, 3, 4 };
    EvalPrint(t.a);
    EvalPrint(t.c);
    ZeroStruct(&t);
    EvalPrint(t.a);
    EvalPrint(t.c);
    
    EvalPrintLine;
    
    EvalPrint(Min(1, 100));
    EvalPrint(Min(100, 20));
    EvalPrint(Max(1, 40));
    EvalPrint(Max(200, 10));
    EvalPrint(Clamp(1, 10, 100));
    EvalPrint(Clamp(80, 0, 7));
    EvalPrint(Clamp(20, 5, 30));
    
    EvalPrintLine;
    
    EvalPrint(IsUpperCase('A'));
    EvalPrint(IsUpperCase('c'));
    EvalPrint(IsUpperCase(';'));
    EvalPrint(IsLowerCase('d'));
    EvalPrint(IsLowerCase('E'));
    EvalPrint(IsLowerCase('0'));
    
    EvalPrintC(UpperCase('A'));
    EvalPrintC(UpperCase('d'));
    EvalPrintC(UpperCase('@'));
    EvalPrintC(LowerCase('a'));
    EvalPrintC(LowerCase('D'));
    EvalPrintC(LowerCase('2'));
    
    EvalPrintLine;
    
    EvalPrint(MIN_I8 );
    EvalPrint(MIN_I16);
    EvalPrint(MIN_I32);
    EvalPrintLL(MIN_I64);
    
    EvalPrintLine;
    
    EvalPrint(MAX_I8 );
    EvalPrint(MAX_I16);
    EvalPrint(MAX_I32);
    EvalPrintLL(MAX_I64);
    
    EvalPrintLine;
    
    EvalPrintU(MAX_U8 );
    EvalPrintU(MAX_U16);
    EvalPrintU(MAX_U32);
    EvalPrintULL(MAX_U64);
    
    EvalPrintLine;
    
    EvalPrintF(EPSILON_F32);
    EvalPrintF(E_F32);
    EvalPrintF(PI_F32);
    EvalPrintF(TAU_F32);
    EvalPrintF(GOLD_BIG_F32);
    EvalPrintF(GOLD_SMALL_F32);
    
    EvalPrintLine;
    
    EvalPrintF(EPSILON_F64);
    EvalPrintF(E_F64);
    EvalPrintF(PI_F64);
    EvalPrintF(TAU_F64);
    EvalPrintF(GOLD_BIG_F64);
    EvalPrintF(GOLD_SMALL_F64);
    
    EvalPrintLine;
    
    EvalPrint(Abs(-3506708));
    EvalPrintLL(AbsI64(MIN_I64 + MAX_I32));
    
    EvalPrintLine;
    
    EvalPrintF(Inf_f32());
    EvalPrintF(NegInf_f32());
    EvalPrintF(Abs_f32(-2.5f));
    
    EvalPrintLine;
    
    EvalPrintF(Inf_f64());
    EvalPrintF(NegInf_f64());
    EvalPrintF(Abs_f64(-357.39460));
    
    EvalPrintLine;
    
    EvalPrintF(Lerp(.75f, 8.5f, 27.3f));
    EvalPrintF(Sqrt_f64(4.));
    EvalPrintF(Cos_f32(30.f));
    EvalPrintF(SinPi64(PI_F64));
    EvalPrintF(SinTurn32(.5f));
    EvalPrintF(SinTurn64(.25));
    
    EvalPrintLine;
    
    EvalPrintF(StrToF32(StrLit("15.75")  , 0));
    EvalPrintF(StrToF32(StrLit("1.575E1"), 0));
    EvalPrintF(StrToF32(StrLit("1575e-2"), 0));
    EvalPrintF(StrToF32(StrLit("-2.5e-3"), 0));
    EvalPrintF(StrToF32(StrLit("25E-4")  , 0));
    EvalPrintF(StrToF32(StrLit(".0075e2"), 0));
    EvalPrintF(StrToF32(StrLit("0.075e1"), 0));
    EvalPrintF(StrToF32(StrLit(".075e1") , 0));
    EvalPrintF(StrToF32(StrLit("75e-2")  , 0));
    
    EvalPrintLine;
    
    EvalPrintLL(StrToI64(StrLit("28"), 0));
    EvalPrintLL(StrToI64(StrLit("4000000024"), 0));
    EvalPrintLL(StrToI64(StrLit("2000000022"), 0));
    EvalPrintLL(StrToI64(StrLit("4000000000"), 0));
    EvalPrintLL(StrToI64(StrLit("9000000000"), 0));
    EvalPrintLL(StrToI64(StrLit("900000000001"), 0));
    EvalPrintLL(StrToI64(StrLit("9000000000002"), 0));
    EvalPrintLL(StrToI64(StrLit("90000000000004"), 0));
    
    EvalPrintLine;
    
    EvalPrintLL(StrToI64(StrLit("024"), 0));
    EvalPrintLL(StrToI64(StrLit("04000000024"), 0));
    EvalPrintLL(StrToI64(StrLit("02000000022"), 0));
    EvalPrintLL(StrToI64(StrLit("04000000000"), 0));
    EvalPrintLL(StrToI64(StrLit("044000000000000"), 0));
    EvalPrintLL(StrToI64(StrLit("044400000000000001"), 0));
    EvalPrintLL(StrToI64(StrLit("04444000000000000002"), 0));
    EvalPrintLL(StrToI64(StrLit("04444000000000000004"), 0));
    
    EvalPrintLine;
    
    EvalPrint(StrToI64(StrLit("0x2a"), 0) == 42);
    EvalPrint(StrToI64(StrLit("0XA0000024"), 0) == 2684354596);
    EvalPrint(StrToI64(StrLit("0x20000022"), 0) == 536870946);
    EvalPrint(StrToI64(StrLit("0XA0000021"), 0) == 2684354593);
    EvalPrint(StrToI64(StrLit("0x8a000000000000"), 0) == 38843546786070528);
    EvalPrint(StrToI64(StrLit("0x8A40000000000010"), 0) == -8484781697966014448);
    EvalPrint(StrToI64(StrLit("0x4a44000000000020"), 0) == 5351402257222991904);
    EvalPrint(StrToI64(StrLit("0x8a44000000000040"), 0) == -8483655798059171776);
    
    EvalPrintLine;
    
    EvalPrint(StrToI64(StrLit("0b10"), 0) == 2);
    EvalPrint(StrToI64(StrLit("0B10000011"), 0) == 131);
    EvalPrint(StrToI64(StrLit("0b100000011"), 0) == 259);
    EvalPrint(StrToI64(StrLit("0B10010100"), 0) == 148);
    EvalPrint(StrToI64(StrLit("0B11111111111111111111111111111111"), 0) == MAX_U32);
    EvalPrint(StrToI64(StrLit("0b0000000000000000000000000000000000000000000000000000000000000000"), 0) == 0);
    EvalPrint(StrToI64(StrLit("0b0000000000000000000000000000000000000000000000000000000000000001"), 0) == 1);
    
    EvalPrintLine;
    
    Arch a = 0;
    EvalPrintS(GetEnumName(Arch, a));
    EvalPrintS(GetEnumName(Arch, Arch_ARM));
    EvalPrintS(GetEnumName(Compiler, CURRENT_COMPILER_NUMBER));
    EvalPrintS(CURRENT_COMPILER_NAME);
    EvalPrintS(CURRENT_ARCH_NAME);
    
    Month month = 7;
    EvalPrintStr(GetEnumStr(Month, month));
    EvalPrintStr(GetEnumStr(Month, 13));
    
    EvalPrint(ArrayCount(Day_names));
    EvalPrintS(GetEnumName(Day, -1));
    
    EvalPrintLine;
    
    MemArena* arena = MakeArena();
    
    InitOSMain(0, 0);
    
    EvalPrintPtr(arena);
    EvalPrintU(arena->cap);
    EvalPrintU(arena->pos);
    EvalPrintU(arena->commitPos);
    
    EvalPrintLine;
    
    TempArena temp = BeginTemp(arena);
    
    u64 arrayCount = 10;
    int* array = PushZeroArray(arena, int, arrayCount);
    for (u64 i = 0; i < arrayCount; ++i)
        EvalPrint(array[i]);
    for (u64 i = 0; i < arrayCount; ++i)
        array[i] = i;
    for (u64 i = 0; i < arrayCount; ++i)
        EvalPrint(array[i]);
    
    EvalPrintPtr(arena);
    EvalPrintU(arena->cap);
    EvalPrintU(arena->pos);
    EvalPrintU(arena->commitPos);
    
    u32 testVal = 1 << 24 | 3;
    f32 testFloat = testVal;
    
    EvalPrintLine;
    
    EndTemp(temp);
    
    EvalPrintPtr(arena);
    EvalPrintU(arena->cap);
    EvalPrintU(arena->pos);
    EvalPrintU(arena->commitPos);
    
    EvalPrintLine;
    
    String str = StrLit("Here's a simple test!");
    EvalPrintStr(str);
    EvalPrint(CompareChar('m', 'M', StringMatchFlag_NoCase));
    EvalPrint(CompareCharArray('M', str, StringMatchFlag_NoCase));
    EvalPrint(CompareCharArray('M', str, 0));
    //EvalPrint(CompareStr(str, StrLit("'s a simple"), StringMatchFlag_Contains));
    //EvalPrint(CompareStr(str, StrLit("'s a simple"), StringMatchFlag_Contains|StringMatchFlag_NotEqual));
    EvalPrint(CompareStr(str, StrLit("Here's a simple tes!"), 0));
    EvalPrint(CompareStr(str, StrLit("Here's a simple tesT!"), StringMatchFlag_NoCase));
    EvalPrint(CompareStr(str, StrLit("!sa"), StringMatchFlag_IsCharArray));
    EvalPrint(CompareStr(str, StrLit("!sa"), StringMatchFlag_NotEqual|StringMatchFlag_IsCharArray));
    
    EvalPrintLine;
    
    String prefix = PrefixStr(str, str.size / 2);
    EvalPrintStr(prefix);
    EvalPrintStr(SkipStr(str, str.size / 2));
    EvalPrintStr(ChopStr(str, 5));
    EvalPrintStr(PostfixStr(str, 2));
    EvalPrintStr(SubStr(str, 2, 8));
    
    EvalPrintLine;
    
    StringList list = SplitStr(arena, str, StrLit(" '"), StringMatchFlag_IsCharArray);
    PushStrList(arena, &list, StrLit("Insert string"));
    for (StringNode* node = list.first; node; node = node->next)
        EvalPrintStr(node->string);
    
    EvalPrintStr(PreSubStr(str, list.first->next->next->next->string));
    EvalPrintStr(PostSubStr(str, list.first->next->next->string));
    
    String newStr = JoinStr(arena, &list, &(StringJoin){ StrLit("a"), StrLit("mc"), StrLit(" mm ") }, true);
    EvalPrintStr(newStr);
    EvalPrintS(newStr.str);
    
    EvalPrintLine;
    
    String word = StrLit("Word");
    String data = PushStrf(arena, true, "%.*s %.*s!\nThe date is %d", StrExpand(StrLit("Hello")), StrExpand(word), 25);
    EvalPrintStr(data);
    
    EvalPrintStr(SkipStrUntil(StrLit("Explain! to me this!"), StrLit("!"), 0));
    EvalPrintStr(SkipStrUntil(StrLit("Explain to me this!"), StrLit("!"), StringMatchFlag_Inclusive));
    EvalPrintStr(ChopStrAfter(StrLit("This's a greate library!"), StrLit("'"), StringMatchFlag_Inclusive));
    EvalPrintStr(ChopStrAfter(StrLit("This's a greate library!"), StrLit("'"), 0));
    
    EvalPrintLine;
    
    String fileData = ReadOSFile(arena, StrLit("data/Test.txt"), true);
    EvalPrintS(fileData.str);
    EvalPrint(WriteOSFile(StrLit("data/Test2.txt"), data));
    
    EvalPrintStr(GetFilePath(arena, SystemPath_CurrentDirectory, false));
    EvalPrintStr(GetFilePath(arena, SystemPath_Binary, false));
    EvalPrintStr(GetFilePath(arena, SystemPath_UserData, false));
    EvalPrintStr(GetFilePath(arena, SystemPath_TempData, false));
    
    EvalPrintLine;
    
    {
        String currentName;
        FileProperties currentProp;
        OSFileIter iter = InitFileIter(StrLit("code"));
        while(NextFileIter(arena, &iter, &currentName, &currentProp, false))
        {
            EvalPrintStr(currentName);
            EvalPrintULL(currentProp.createTime);
            EvalPrintULL(currentProp.modifyTime);
            EvalPrint(ToDateTime(currentProp.modifyTime).year);
        }
    }
    
    EvalPrintLine;
    
    u64 entropy;
    GetOSEntropy(&entropy, sizeof(entropy));
    EvalPrintULL(entropy);
    
    EvalPrintLine;
    
    OSLib testLib = LoadOSLib(StrLit("build/TestDLL.dll"));
    EvalPrintPtr(testLib.v);
    u32(*func)(u32*, u64) = (u32 (*)(u32*, u64))GetOSProc(testLib, "Sum");
    EvalPrintU(func((u32*)&entropy, 2));
    i32* dllVar = (i32*)GetOSProc(testLib, "globalInt");
    EvalPrint(*dllVar);
    
    RenameOSFile(StrLit("build/TestDLL.dll"), StrLit("build/NewTestDLL.dll"));
    
    FileProperties file = GetFileProperties(StrLit("build/NewTestDLL.dll"));
    EvalPrintULL(file.createTime);
    EvalPrintULL(file.modifyTime);
    EvalPrint(ToDateTime(file.modifyTime).year);
    
    EvalPrintLine;
    
    EvalPrintULL(NowMicroseconds());
    DateTime now = NowUniversalTime();
    EvalPrint(now.msec);
    EvalPrint(ToLocalTime(&now).msec);
    
    EvalPrintU(arena->highWaterMark);
    ReleaseArena(arena);
    
    return 0;
}
#elif CODE_PATH == 1
#include "LongGFX.h"
#include "LongGFX_Win32.c"
#include "LongGFX_OpenGL.h"
#include "Win32_OpenGL.c"
#include "LongGFX_D3D11.h"
#include "Win32_D3D11.c"

enum
{
	Renderer_GL,
	Renderer_D3D,
};

function void WindowResizeHandler(GFXWindow window, u32 width, u32 height)
{
	u32 renderer = IntFromPtr(GetGFXWindowUserData(window));
	switch (renderer)
	{
		case Renderer_GL:
		{
			BeginGLRender(window);
			
			if (IsGFXWindowMaximized(window))
				glClearColor(0.f, 1.f, 1.f, 1.f);
			else if (IsGFXWindowMinimized(window))
				glClearColor(1.f, 0.f, 1.f, 1.f);
			else if (IsGFXWindowFullScreen(window))
				glClearColor(1.f, 1.f, 0.f, 1.f);
			else
				glClearColor(0.f, 0.f, 1.f, 1.f);
			
			glClear(GL_COLOR_BUFFER_BIT);
			EndGLRender();
		} break;
		case Renderer_D3D:
		{
			ID3D11RenderTargetView* view = BeginD3D11Render(window);
			ID3D11DeviceContext_ClearRenderTargetView(ctx, view, MidnightBlue);
			ID3D11DeviceContext_OMSetRenderTargets(ctx, 1, &view, 0);
			EndD3D11Render(view);
		} break;
	}
}

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    BeginScratch(scratch);
    
    if (!InitGFX())
        GFXErrorBox(1);
    
    if (!InitGL())
        GFXErrorBox(1);
    
    if (!InitD3D11())
        GFXErrorBox(1);
    
	SetGFXResizeFunc(WindowResizeHandler);
	
#define TEST_WINDOW_COUNT 4
	GFXWindow windows[TEST_WINDOW_COUNT] = {0};
	for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
	{
		i32 w = 400, h = 200;
		/*if (i > 0)
        GetGFXWindowInnerRect(windows[i - 1], 0, 0, &w, &h);*/
		w = w * (i + 1);
		h = h * (i + 1);
		windows[i] = CreateGFXWindowEx(PushStrf(scratch, true, "Window: %d", i), CW_USEDEFAULT, CW_USEDEFAULT, w, h);
		i32 isGL = i % 2;
		if (isGL == 0)
			EquipGLWindow(windows[i]);
		else
			EquipD3D11Window(windows[i]);
		if (!windows[i])
			GFXErrorBox(1);
		SetGFXWindowUserData(windows[i], PtrFromInt(isGL));
		ShowGFXWindow(windows[i]);
	}
	
    ID3D11DeviceContext* ctx = GetD3D11DeviceCtx();
    
	u32 count = 0;
	b32 visible = false;
    for (;;)
    {
        if (!WaitForGFXInput())
			break;
        
		//OutputDebugString(PushStrf(scratch, true, "Count: %d\n", count).str);
		count++;
		if (count % 50 == 0)
		{
			if (false)
			{
				GFXWindow window = windows[TEST_WINDOW_COUNT-1];
				if (IsGFXWindowValid(window))
				{
					SetGFXWindowVisible(window, visible);
					if (visible)
						SetGFXWindowTitle(window, PushStrf(scratch, true, "Toggle Windows: %d", count / 100));
					visible = !visible;
				}
			}
			
			if (false)
			{
				GFXWindow window = windows[TEST_WINDOW_COUNT-2];
				if (IsGFXWindowValid(window))
				{
					i32 x, y, w, h;
					GetGFXWindowOuterRect(window, &x, &y, &w, &h);
					SetGFXWindowInnerRect(window, x, y, w, h);
				}
			}
			
			if (false)
			{
				GFXWindow window = windows[TEST_WINDOW_COUNT-2];
				if (IsGFXWindowValid(window))
					SetGFXWindowFullScreen(window, count % 100 == 0);
			}
			
			{
				GFXWindow window = windows[1];
				if (IsGFXWindowValid(window))
				{
					OutputDebugString(PushStrf(scratch, true, "Before: %d\n", IsGFXWindowResizable(window)).str);
					SetGFXWindowResizable(window, count % 100);
					OutputDebugString(PushStrf(scratch, true, "After: %d\n", IsGFXWindowResizable(window)).str);
				}
			}
		}
		
		u32 activeWindowCount = 0;
		for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
		{
			if (IsGFXWindowValid(windows[i]))
			{
				activeWindowCount++;
				WindowResizeHandler(windows[i], 0, 0);
				if (IsGFXWindowMinimized(windows[i]))
					CloseGFXWindow(windows[i]);
			}
		}
		
		if (!activeWindowCount)
			break;
	}
	
	{
		ID3D11Device_Release(device);
		ID3D11DeviceContext_Release(ctx);
		ID3D11Debug_Release(dbg);
	}
	
	EndScratch(scratch);
	return 0;
}
#endif