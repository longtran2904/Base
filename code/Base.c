
//~/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////         BASE IMPLEMENT         ////////////////
////////////////////////////////////////////////////////////////
//-/////////////////////////////////////////////////////////////

//~ long: Symbolic Constants

readonly global const String Compiler_names[] =
{
    StrConst("None"),
    StrConst("CLANG"),
    StrConst("CL"),
    StrConst("GCC"),
};

readonly global const String Arch_names[] =
{
    StrConst("None"),
    StrConst("X64"),
    StrConst("X86"),
    StrConst("ARM"),
    StrConst("ARM64"),
};

readonly global const String OS_names[] =
{
    StrConst("None"),
    StrConst("WIN"),
    StrConst("LINUX"),
    StrConst("MAC"),
};

readonly global const String Month_names[] =
{
    StrConst("Jan"),
    StrConst("Feb"),
    StrConst("Mar"),
    StrConst("Apr"),
    StrConst("May"),
    StrConst("Jun"),
    StrConst("Jul"),
    StrConst("Aug"),
    StrConst("Sep"),
    StrConst("Oct"),
    StrConst("Nov"),
    StrConst("Dec"),
};

readonly global const String Day_names[] =
{
    StrConst("Sunday"),
    StrConst("Monday"),
    StrConst("Tuesday"),
    StrConst("Wednesday"),
    StrConst("Thursday"),
    StrConst("Friday"),
    StrConst("Saturday"),
};

//~ long: Built-in Functions

#if COMPILER_CL || (COMPILER_CLANG && OS_WIN) // https://stackoverflow.com/a/76705994
#include <intrin.h>
function u32 __ctz32(u32 x) { u32 result; _BitScanForward  (&result, x); return result; }
function u64 __ctz64(u64 x) { u32 result; _BitScanForward64(&result, x); return result; }
function u32 __clz32(u32 x) { return __lzcnt  (x); }
function u64 __clz64(u64 x) { return __lzcnt64(x); }
#else

//- long: Bit Counting
function u32 __ctz32(u32 x) { return (__builtin_ctz  (x)); }
function u64 __ctz64(u64 x) { return (__builtin_ctzll(x)); }
function u32 __clz32(u32 x) { return (__builtin_clz  (x)); }
function u64 __clz64(u64 x) { return (__builtin_clzll(x)); }

function u16 __popcnt16(u16 x) { return (u16)__builtin_popcount  (x); }
function u32 __popcnt  (u32 x) { return      __builtin_popcount  (x); }
function u64 __popcnt64(u64 x) { return      __builtin_popcountll(x); }

//- long: REP MOV
// https://hero.handmade.network/forums/code-discussion/t/157-memory_bandwidth_+_implementing_memcpy
#define INLINE_ASM(ins) __asm__ __volatile__(ins : "+D"(dst), "+S"(src), "+c"(size) : : "memory")

function void __movsb(void* dst, const void* src, uptr size) { INLINE_ASM("rep movsb"); }
function void __movsd(void* dst, const void* src, uptr size) { INLINE_ASM("rep movsl"); }
function void __movsq(void* dst, const void* src, uptr size) { INLINE_ASM("rep movsq"); }
#endif

function u32 clz32(u32 x) { return x ? __clz32(x) : 32; }
function u64 clz64(u64 x) { return x ? __clz64(x) : 64; }
function u32 ctz32(u32 x) { return x ? __ctz32(x) : 32; }
function u64 ctz64(u64 x) { return x ? __ctz64(x) : 64; }

//~ long: Math Functions

//- long: Trigonometric Functions
#ifndef EXTRA_PRECISION
#define EXTRA_PRECISION
#endif

// https://web.archive.org/web/20060503192740/http://www.devmaster.net/forums/showthread.php?t=5784
internal f64 SinPi64(f64 x)
{
    f64 y = 4 / PI_F64 * x + -4 / (PI_F64 * PI_F64) * x * Abs_f64(x);
    
#ifdef EXTRA_PRECISION
    //  f64 Q = 0.775;
    f64 P = 0.225;
    y = P * (y * Abs_f64(y) - y) + y;   // Q * y + P * y * abs(y)
#endif
    
    return y;
}

internal f64 SinTurn64(f64 x)
{
    f64 y = 4 * x + -4 * x * Abs_f64(x);
#ifdef EXTRA_PRECISION
    y = 0.225 * (y * Abs_f64(y) - y) + y;
#endif
    return y;
}
#undef EXTRA_PRECISION

internal f32 SinPi32(f32 x)
{
    return (f32)SinPi64((f64)x);
}

internal f32 SinTurn32(f32 x)
{
    return (f32)SinTurn64((f64)x);
}

internal f64 CosPi64(f64 x)
{
    x += PI_F64 / 2;
    x -= (x > PI_F64) * (2 * PI_F64);
    return SinTurn64(x);
}

#include <math.h>

function f32 Sin_f32(f32 x) { return sinf(x); }
function f32 Cos_f32(f32 x) { return cosf(x); }
function f32 Tan_f32(f32 x) { return tanf(x); }

function f64 Sin_f64(f64 x) { return sin(x); }
function f64 Cos_f64(f64 x) { return cos(x); }
function f64 Tan_f64(f64 x) { return tan(x); }

function f32 Atan_f32(f32 x) { return atanf(x); }
function f64 Atan_f64(f64 x) { return atan (x); }

function f32 Atan2_f32(f32 x, f32 y) { return atan2f(x, y); }
function f64 Atan2_f64(f64 x, f64 y) { return atan2 (x, y); }

//- long: Float Constant Functions
function f32 Inf_f32(void)
{
    f32 f = 0;
    u32 u = *(u32*)&f;
    u = 0x7f800000;
    return *(f32*)&u;
}

function f32 NegInf_f32(void)
{
    f32 f = 0;
    u32 u = *(u32*)&f;
    u = 0xff800000;
    return *(f32*)&u;
}

function b32 InfOrNan_f32(f32 x)
{
    u32 u = *(u32*)&x;
    return (u & 0x7f800000) == 0x7f800000;
}

function f64 Inf_f64(void)
{
    f64 f = 0;
    u64 u = *(u64*)&f;
    u = 0x7ff0000000000000;
    return *(f64*)&u;
}

function f64 NegInf_f64(void)
{
    f64 f = 0;
    u64 u = *(u64*)&f;
    u = 0xfff0000000000000;
    return *(f64*)&u;
}

function b32 InfOrNan_f64(f64 x)
{
    u64 u = *(u64*)&x;
    return (u & 0x7ff0000000000000) == 0x7ff0000000000000;
}

function i32 AbsI32(i32 x)
{
    i32 mask = x >> 31;
    return (x ^ mask) - mask; // or (n + mask) ^ mask
}
function i64 AbsI64(i64 x)
{
    i64 mask = x >> 63;
    return (x ^ mask) - mask; // or (n + mask) ^ mask
}

function f32 Abs_f32(f32 x)
{
    u32 u = *(u32*)&x;
    u &= 0x7fffffff;
    return *(f32*)&u;
}
function f64 Abs_f64(f64 x)
{
    u64 u = *(u64*)&x;
    u &= 0x7fffffffffffffff; // 1 + epsilon != 1
    return *(f64*)&u;
}

//- long: Scalar Functions
#ifdef __SSE4_1__
#include <smmintrin.h>

// NOTE(long): _mm_floor/ceil_ss is just macro that expand into _MM_FROUND_TO_NEG/POS_INF|_MM_FROUND_NO_EXC
#define ROUND_F32(x, flags) _mm_cvtss_f32(_mm_round_ss((__m128){0}, _mm_set_ss(x), flags))
function f32 Round_f32(f32 x) { return ROUND_F32(x, _MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC); }
function f32 Trunc_f32(f32 x) { return ROUND_F32(x,        _MM_FROUND_TO_ZERO|_MM_FROUND_NO_EXC); }
function f32 Floor_f32(f32 x) { return ROUND_F32(x,     _MM_FROUND_TO_NEG_INF|_MM_FROUND_NO_EXC); }
function f32  Ceil_f32(f32 x) { return ROUND_F32(x,     _MM_FROUND_TO_POS_INF|_MM_FROUND_NO_EXC); }

#define ROUND_F64(x, flags) _mm_cvtsd_f64(_mm_round_sd((__m128d){0}, _mm_set_sd(x), flags))
function f64 Round_f64(f64 x) { return ROUND_F64(x, _MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC); }
function f64 Trunc_f64(f64 x) { return ROUND_F64(x,        _MM_FROUND_TO_ZERO|_MM_FROUND_NO_EXC); }
function f64 Floor_f64(f64 x) { return ROUND_F64(x,     _MM_FROUND_TO_NEG_INF|_MM_FROUND_NO_EXC); }
function f64  Ceil_f64(f64 x) { return ROUND_F64(x,     _MM_FROUND_TO_POS_INF|_MM_FROUND_NO_EXC); }

#elif defined(__SSE2__)
#include <emmintrin.h>

#define ROUND_F32(x, ...) \
    __m128 f = _mm_set_ss(x); \
    __m128 r = _mm_cvtepi32_ps(_mm_cvttps_epi32(f)); /*r = (f32)(i32)f*/ \
    __VA_ARGS__; \
    __m128 m = _mm_cmpgt_ss(_mm_set1_ps(0x1p31f), _mm_andnot_ps(_mm_set1_ps(-0.f), f)); \
    r = _mm_or_ps(_mm_and_ps(m, r), _mm_andnot_ps(m, f)); /*if (!(2**31 > abs(f))) r = f;*/ \
    x = _mm_cvtss_f32(r)

function f32 Round_f32(f32 x)
{
    __m128 f = _mm_set_ss(x);
    __m128 max_mantissa = _mm_set1_ps(x > 0 ? 0x1p23f : -0x1p23f);
    __m128 r = _mm_sub_ss(_mm_add_ss(f, max_mantissa), max_mantissa); // f + 0x1p23f - 0x1p23f;
    __m128 m = _mm_cmpgt_ss(_mm_set1_ps(0x1p31f), _mm_andnot_ps(_mm_set1_ps(-0.f), f));
    r = _mm_or_ps(_mm_and_ps(m, r), _mm_andnot_ps(m, f)); //if (!(2**31 > abs(f))) r = f;
    return _mm_cvtss_f32(r);
}

function f32 Trunc_f32(f32 x)
{
    ROUND_F32(x);
    return x;
}

function f32 Floor_f32(f32 x)
{
    ROUND_F32(x, r = _mm_sub_ss(r, _mm_and_ps(_mm_cmplt_ss(f, r), _mm_set1_ps(1.f)))); // if (f < r) r -= 1
    return x;
}

function f32 Ceil_f32(f32 x)
{
    ROUND_F32(x, r = _mm_add_ss(r, _mm_and_ps(_mm_cmpgt_ss(f, r), _mm_set1_ps(1.f)))); // if (f > r) r += 1
    return x;
}

#define ROUND_F64(x, ...) \
    __m128d f = _mm_set_sd(x); \
    __m128d r = _mm_cvtsi64_sd((__m128d){0}, _mm_cvttsd_si64(f)); /*r = (f64)(i64)f*/ \
    __VA_ARGS__; \
    __m128d m = _mm_cmpgt_sd(_mm_set1_pd(0x1p63), _mm_andnot_pd(_mm_set1_pd(-0.), f)); \
    r = _mm_or_pd(_mm_and_pd(m, r), _mm_andnot_pd(m, f)); /*if (!(2**63 > abs(f))) r = f;*/ \
    x = _mm_cvtsd_f64(r);

function f64 Round_f64(f64 x)
{
    __m128d f = _mm_set_sd(x);
    __m128d max_mantissa = _mm_set1_pd(x > 0 ? 0x1p52 : -0x1p52);
    __m128d r = _mm_sub_sd(_mm_add_sd(f, max_mantissa), max_mantissa);
    __m128d m = _mm_cmpgt_sd(_mm_set1_pd(0x1p63), _mm_andnot_pd(_mm_set1_pd(-0.), f));
    r = _mm_or_pd(_mm_and_pd(m, r), _mm_andnot_pd(m, f)); //if (!(2**63 > abs(f))) r = f;
    return _mm_cvtsd_f64(r);
}

function f64 Trunc_f64(f64 x)
{
    ROUND_F64(x);
    return x;
}

function f64 Floor_f64(f64 x)
{
    ROUND_F64(x, r = _mm_sub_sd(r, _mm_and_pd(_mm_cmplt_sd(f, r), _mm_set1_pd(1.)))); // if (f < r) r -= 1
    return x;
}

function f64 Ceil_f64(f64 x)
{
    ROUND_F64(x, r = _mm_add_sd(r, _mm_and_pd(_mm_cmpgt_sd(f, r), _mm_set1_pd(1.)))); // if (f > r) r += 1
    return x;
}
#endif
#undef ROUND_F32
#undef ROUND_F64

function f32 Sqrt_f32(f32 x) { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x))); }
function f64 Sqrt_f64(f64 x) { return _mm_cvtsd_f64(_mm_sqrt_sd((__m128d){0}, _mm_set_sd(x))); }
function f32 RSqrt_f32(f32 x) { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }

function f32 Ln_f32(f32 x) { return logf(x); }
function f64 Ln_f64(f64 x) { return log (x); }

function f32 Pow_f32(f32 base, f32 x) { return powf(base, x); }
function f64 Pow_f64(f64 base, f64 x) { return pow (base, x); }

function f32 FrExp_f32(f32 x, i32* exp) { return frexpf(x, exp); }
function f64 FrExp_f64(f64 x, i32* exp) { return frexp (x, exp); }

//- long: Vector Functions
function v2f32 V2F32(f32 x, f32 y)                { return (v2f32){ x, y }; }
function v2f32 AddV2F32(v2f32 a, v2f32 b)         { return (v2f32){ a.x + b.x, a.y + b.y }; }
function v2f32 SubV2F32(v2f32 a, v2f32 b)         { return (v2f32){ a.x - b.x, a.y - b.y }; }
function v2f32 MulV2F32(v2f32 a, v2f32 b)         { return (v2f32){ a.x * b.x, a.y * b.y }; }
function v2f32 DivV2F32(v2f32 a, v2f32 b)         { return (v2f32){ a.x / b.x, a.y / b.y }; }
function v2f32 ScaleV2F32(v2f32 v, f32 s)         { return (v2f32){ v.x * s, v.y * s }; }
function f32 DotV2F32(v2f32 a, v2f32 b)           { return a.x * b.x + a.y * b.y; }
function f32 SqrMagV2F32(v2f32 v)                 { return v.x * v.x + v.y * v.y; }
function f32 MagV2F32(v2f32 v)                    { return Sqrt_f32(v.x * v.x + v.y * v.y); }
function f32 AngleV2F32(v2f32 v)                { return Atan2_f32(v.x, v.y); }
function v2f32 V2F32Angle(f32 theta, f32 radius)  { return (v2f32){ radius*Cos_f32(theta), radius*Sin_f32(theta) }; }
function v2f32 NormV2F32(v2f32 v)                 { return ScaleV2F32(v, 1.f/MagV2F32(v)); }
function v2f32 LerpV2F32(v2f32 a, v2f32 b, f32 t) { return (v2f32){ Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) }; }

function v2i32 V2I32(i32 x, i32 y)                { return (v2i32){ x, y }; }
function v2i32 AddV2I32(v2i32 a, v2i32 b)         { return (v2i32){ a.x + b.x, a.y + b.y }; }
function v2i32 SubV2I32(v2i32 a, v2i32 b)         { return (v2i32){ a.x - b.x, a.y - b.y }; }
function v2i32 MulV2I32(v2i32 a, v2i32 b)         { return (v2i32){ a.x * b.x, a.y * b.y }; }
function v2i32 DivV2I32(v2i32 a, v2i32 b)         { return (v2i32){ a.x / b.x, a.y / b.y }; }
function v2i32 ScaleV2I32(v2i32 v, i32 s)         { return (v2i32){ v.x * s, v.y * s }; }
function i32 DotV2I32(v2i32 a, v2i32 b)           { return a.x * b.x + a.y * b.y; }
function i32 SqrMagV2I32(v2i32 v)                 { return v.x * v.x + v.y * v.y; }
function i32 MagV2I32(v2i32 v)                    { return (i32)Sqrt_f32((f32)(v.x * v.x + v.y * v.y)); }
function v2i32 NormV2I32(v2i32 v)                 { return ScaleV2I32(v, (i32)(1.f/(f32)MagV2I32(v))); }
function v2i32 LerpV2I32(v2i32 a, v2i32 b, f32 t) { return (v2i32){ LerpInt(a.x, b.x, t), LerpInt(a.y, b.y, t) }; }

function v2i64 V2I64(i64 x, i64 y)                { return (v2i64){ x, y }; }
function v2i64 AddV2I64(v2i64 a, v2i64 b)         { return (v2i64){ a.x + b.x, a.y + b.y }; }
function v2i64 SubV2I64(v2i64 a, v2i64 b)         { return (v2i64){ a.x - b.x, a.y - b.y }; }
function v2i64 MulV2I64(v2i64 a, v2i64 b)         { return (v2i64){ a.x * b.x, a.y * b.y }; }
function v2i64 DivV2I64(v2i64 a, v2i64 b)         { return (v2i64){ a.x / b.x, a.y / b.y }; }
function v2i64 ScaleV2I64(v2i64 v, i64 s)         { return (v2i64){ v.x * s, v.y * s }; }
function i64 DotV2I64(v2i64 a, v2i64 b)           { return a.x * b.x + a.y * b.y; }
function i64 SqrMagV2I64(v2i64 v)                 { return v.x * v.x + v.y * v.y; }
function i64 MagV2I64(v2i64 v)                    { return (i64)Sqrt_f32((f32)(v.x * v.x + v.y * v.y)); }
function v2i64 NormV2I64(v2i64 v)                 { return ScaleV2I64(v, (i64)(1.f/(f32)MagV2I64(v))); }
function v2i64 LerpV2I64(v2i64 a, v2i64 b, f32 t) { return (v2i64){ LerpInt(a.x, b.x, t), LerpInt(a.y, b.y, t) }; }

function v3f32 V3F32(f32 x, f32 y, f32 z)         { return (v3f32){ x, y, z }; }
function v3f32 AddV3F32(v3f32 a, v3f32 b)         { return (v3f32){ a.x + b.x, a.y + b.y, a.z + b.z }; }
function v3f32 SubV3F32(v3f32 a, v3f32 b)         { return (v3f32){ a.x - b.x, a.y - b.y, a.z - b.z }; }
function v3f32 MulV3F32(v3f32 a, v3f32 b)         { return (v3f32){ a.x * b.x, a.y * b.y, a.z * b.z }; }
function v3f32 DivV3F32(v3f32 a, v3f32 b)         { return (v3f32){ a.x / b.x, a.y / b.y, a.z / b.z }; }
function v3f32 ScaleV3F32(v3f32 v, f32 s)         { return (v3f32){ v.x * s, v.y * s, v.z * s }; }
function f32 DotV3F32(v3f32 a, v3f32 b)           { return a.x * b.x + a.y * b.y + a.z * b.z; }
function f32 SqrMagV3F32(v3f32 v)                 { return v.x * v.x + v.y * v.y + v.z * v.z; }
function f32 MagV3F32(v3f32 v)                    { return Sqrt_f32(v.x * v.x + v.y * v.y + v.z * v.z); }
function v3f32 NormV3F32(v3f32 v)                 { return ScaleV3F32(v, 1.f/MagV3F32(v)); }
function v3f32 LerpV3F32(v3f32 a, v3f32 b, f32 t) { return (v3f32){ Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) }; }
function v3f32 CrossV3F32(v3f32 a, v3f32 b)       { return (v3f32){ a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }

function v3i32 V3I32(i32 x, i32 y, i32 z)         { return (v3i32){ x, y, z }; }
function v3i32 AddV3I32(v3i32 a, v3i32 b)         { return (v3i32){ a.x + b.x, a.y + b.y, a.z + b.z }; }
function v3i32 SubV3I32(v3i32 a, v3i32 b)         { return (v3i32){ a.x - b.x, a.y - b.y, a.z - b.z }; }
function v3i32 MulV3I32(v3i32 a, v3i32 b)         { return (v3i32){ a.x * b.x, a.y * b.y, a.z * b.z }; }
function v3i32 DivV3I32(v3i32 a, v3i32 b)         { return (v3i32){ a.x / b.x, a.y / b.y, a.z / b.z }; }
function v3i32 ScaleV3I32(v3i32 v, i32 s)         { return (v3i32){ v.x * s, v.y * s, v.z * s }; }
function i32 DotV3I32(v3i32 a, v3i32 b)           { return a.x * b.x + a.y * b.y + a.z * b.z; }
function i32 SqrMagV3I32(v3i32 v)                 { return v.x * v.x + v.y * v.y + v.z * v.z; }
function i32 MagV3I32(v3i32 v)                    { return (i32)Sqrt_f32((f32)(v.x * v.x + v.y * v.y + v.z * v.z)); }
function v3i32 NormV3I32(v3i32 v)                 { return ScaleV3I32(v, (i32)(1.f/(f32)MagV3I32(v))); }
function v3i32 LerpV3I32(v3i32 a, v3i32 b, f32 t) { return (v3i32){ LerpInt(a.x, b.x, t), LerpInt(a.y, b.y, t), LerpInt(a.z, b.z, t) }; }
function v3i32 CrossV3I32(v3i32 a, v3i32 b)       { return (v3i32){ a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }

//- long: Range Functions
function r1i32 R1I32(i32 min, i32 max)          { return (r1i32){ min, max }; }
function r1i32 R1I32Size(i32 min, i32 size)     { return R1I32(min, min + size); }
function r1i32 ShiftR1I32(r1i32 r, i32 x)       { r.min += x; r.max += x; return r; }
function r1i32 PadR1I32(r1i32 r, i32 x)         { r.min += x; r.max -= x; return r; }
function i32 CenterR1I32(r1i32 r)               { return (r.min + r.max) / 2; }
function b32 ContainsR1I32(r1i32 r, i32 x)      { return r.min <= x && x < r.max; }
function i32 DimR1I32(r1i32 r)                  { return r.max - r.min; }
function r1i32 UnionR1I32(r1i32 a, r1i32 b)     { return (r1i32){ Min(a.min, b.min), Max(a.max, b.max) }; }
function r1i32 IntersectR1I32(r1i32 a, r1i32 b) { return (r1i32){ Max(a.min, b.min), Min(a.max, b.max) }; }
function i32 ClampR1I32(r1i32 r, i32 v)         { return Clamp(v, r.min, r.max); }

function r1u64 R1U64(u64 min, u64 max)          { return (r1u64){ min, max }; }
function r1u64 R1U64Size(u64 min, u64 size)     { return R1U64(min, min + size); }
function r1u64 ShiftR1U64(r1u64 r, u64 x)       { r.min += x; r.max += x; return r; }
function r1u64 PadR1U64(r1u64 r, u64 x)         { r.min += x; r.max -= x; return r; }
function u64 CenterR1U64(r1u64 r)               { return (r.min + r.max) / 2; }
function b32 ContainsR1U64(r1u64 r, u64 x)      { return r.min <= x && x < r.max; }
function u64 DimR1U64(r1u64 r)                  { return r.max - r.min; }
function r1u64 UnionR1U64(r1u64 a, r1u64 b)     { return (r1u64){ Min(a.min, b.min), Max(a.max, b.max) }; }
function r1u64 IntersectR1U64(r1u64 a, r1u64 b) { return (r1u64){ Max(a.min, b.min), Min(a.max, b.max) }; }
function u64 ClampR1U64(r1u64 r, u64 v)         { return Clamp(v, r.min, r.max); }

function r1f32 R1F32(f32 min, f32 max)          { return (r1f32){ min, max }; }
function r1f32 R1F32Size(f32 min, f32 size)     { return R1F32(min, min + size); }
function r1f32 ShiftR1F32(r1f32 r, f32 x)       { r.min += x; r.max += x; return r; }
function r1f32 PadR1F32(r1f32 r, f32 x)         { r.min += x; r.max -= x; return r; }
function f32 CenterR1F32(r1f32 r)               { return (r.min + r.max) / 2.f; }
function b32 ContainsR1F32(r1f32 r, f32 x)      { return r.min <= x && x < r.max; }
function f32 DimR1F32(r1f32 r)                  { return r.max - r.min; }
function r1f32 UnionR1F32(r1f32 a, r1f32 b)     { return (r1f32){ Min(a.min, b.min), Max(a.max, b.max) }; }
function r1f32 IntersectR1F32(r1f32 a, r1f32 b) { return (r1f32){ Max(a.min, b.min), Min(a.max, b.max) }; }
function f32 ClampR1F32(r1f32 r, f32 v)         { return Clamp(v, r.min, r.max); }

function r2i32 R2I32(v2i32 min, v2i32 max)      { return (r2i32){ min, max }; }
function r2i32 R2I32Size(v2i32 min, v2i32 size) { return R2I32(min, AddV2I32(min, size)); }
function r2i32 ShiftR2I32(r2i32 r, v2i32 x)     { r.min = AddV2I32(r.min, x); r.max = AddV2I32(r.max, x); return r; }
function r2i32 PadR2I32(r2i32 r, i32 x)         { r.x0 += x; r.x1 += x; r.y0 += x; r.y1 += x; return r; }
function v2i32 CenterR2I32(r2i32 r)             { return (v2i32){ (r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2 }; }
function b32 ContainsR2I32(r2i32 r, v2i32 x)    { return r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y; }
function v2i32 DimR2I32(r2i32 r)                { return (v2i32){ r.max.x - r.min.x, r.max.y - r.min.y }; }
function r2i32 UnionR2I32(r2i32 a, r2i32 b)     { return (r2i32){ Min(a.x0, b.x0), Min(a.y0, b.y0), Max(a.x1, b.x1), Max(a.y1, b.y1) }; }
function r2i32 IntersectR2I32(r2i32 a, r2i32 b) { return (r2i32){ Max(a.x0, b.x0), Max(a.y0, b.y0), Min(a.x1, b.x1), Min(a.y1, b.y1) }; }
function v2i32 ClampR2I32(r2i32 r, v2i32 v)     { return (v2i32){ Clamp(v.x, r.min.x, r.max.x), Clamp(v.y, r.min.y, r.max.y) }; }

function r2f32 R2F32(v2f32 min, v2f32 max)      { return (r2f32){ min, max }; }
function r2f32 R2F32Size(v2f32 min, v2f32 size) { return R2F32(min, AddV2F32(min, size)); }
function r2f32 ShiftR2F32(r2f32 r, v2f32 x)     { r.min = AddV2F32(r.min, x); r.max = AddV2F32(r.max, x); return r; }
function r2f32 PadR2F32(r2f32 r, f32 x)         { r.x0 += x; r.x1 += x; r.y0 += x; r.y1 += x; return r; }
function v2f32 CenterR2F32(r2f32 r)             { return (v2f32){ (r.min.x + r.max.x)/2.f, (r.min.y + r.max.y)/2.f }; }
function b32 ContainsR2F32(r2f32 r, v2f32 x)    { return r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y; }
function v2f32 DimR2F32(r2f32 r)                { return (v2f32){ r.max.x - r.min.x, r.max.y - r.min.y }; }
function r2f32 UnionR2F32(r2f32 a, r2f32 b)     { return (r2f32){ Min(a.x0, b.x0), Min(a.y0, b.y0), Max(a.x1, b.x1), Max(a.y1, b.y1) }; }
function r2f32 IntersectR2F32(r2f32 a, r2f32 b) { return (r2f32){ Max(a.x0, b.x0), Max(a.y0, b.y0), Min(a.x1, b.x1), Min(a.y1, b.y1) }; }
function v2f32 ClampR2F32(r2f32 r, v2f32 v)     { return (v2f32){ Clamp(v.x, r.min.x, r.max.x), Clamp(v.y, r.min.y, r.max.y) }; }

//- long: Text Functions
function TextLoc TextLocFromOff(String str, u64 off)
{
    TextLoc result = {1, 1};
    for (u64 i = 0; i < str.size && i < off; ++i)
    {
        result.col++;
        if (str.str[i] == '\n')
        {
            result.line++;
            result.col = 1;
        }
    }
    return result;
}

function u64 OffFromTextLoc(String str, TextLoc loc)
{
    u64 result = 0;
    for (u64 i = 0, line = 1, col = 0; i < str.size; i++)
    {
        col++;
        
        if (line == loc.line && col == loc.col)
        {
            result = i;
            break;
        }
        
        if (str.str[i] == '\n')
        {
            if (line == loc.line)
                break;
            
            line++;
            col = 1;
        }
    }
    return result;
}

//~ long: PRNG Functions

function u32 Noise1D(u32 pos, u32 seed)
{
    u32 result = pos;
    result *= BIT_NOISE1;
    result += seed;
    result ^= (result >> 8);
    result += BIT_NOISE2;
    result ^= (result << 8);
    result *= BIT_NOISE3;
    result ^= (result >> 8);
    return result;
}

function u64 Hash64(u8* values, u64 count)
{
    u64 result = BIT_NOISE5;
    for (u64 i = 0; i < count / 8; ++i)
    {
        u64 value = *((u64*)values + i);
        u32 hi = Noise1D((u32)i, (u32)(value >> (i % 16)));
        u32 lo = Noise1D((u32)i, (u32)(value >> (i % 16)));
        result = hi|(((u64)lo) << 32);
    }
    
    for (u8* i = values + (count % 8); i < values + count; ++i)
        result += BIT_NOISE4 * (*i) + BIT_NOISE5;
    return result;
}

//~ long: Time

function DateTime TimeToDate(DenseTime time)
{
    DateTime result = {0};
    result.msec = time % 1000;
    time /= 1000;
    result.sec = time % 61;
    time /= 61;
    result.min = time % 60;
    time /= 60;
    result.hour = time%24;
    time /= 24;
    result.day = time % 31;
    time /= 31;
    result.mon = time % 12;
    time /= 12;
    i32 encodedYear = (i32)time;
    result.year = (i16)(encodedYear - 0x8000);
    return result;
}

function DenseTime TimeToDense(DateTime time)
{
    DenseTime result = 0;
    u32 encodedYear = (u32)((i32)time.year + 0x8000);
    result+= encodedYear;
    result *= 12;
    result += time.mon;
    result *= 31;
    result += time.day;
    result *= 24;
    result += time.hour;
    result *= 60;
    result += time.min;
    result *= 61;
    result += time.sec;
    result *= 1000;
    result += time.msec;
    return result;
}

//~ long: Arena Functions

#define ArenaMinSize(arena) AlignUpPow2(sizeof(Arena), (arena)->alignment)

function Arena* BufferFromMem(u8* mem, u64 size)
{
    Arena* result = 0;
    
    if (ALWAYS(size >= sizeof(Arena)))
    {
        result = (Arena*)mem;
        *result = (Arena)
        {
            .curr = result,
            .alignment = 1,
            .commitPos = size,
            .cap = size,
        };
        
        u64 minPos = ArenaMinSize(result);
        result->pos = minPos;
        AsanPoison(result, size);
        AsanUnpoison(result, minPos);
    }
    
    return result;
}

function Arena* BufferFromArena(Arena* arena, u64 size)
{
    u64 bufferSize = size + sizeof(Arena);
    u8* buffer = ArenaPush(arena, bufferSize);
    return BufferFromMem(buffer, bufferSize);
}

function Arena* ArenaReserve(u64 reserve, u32 alignment, b32 growing)
{
    Arena* result = 0;
    if (reserve >= MEM_INITIAL_COMMIT)
    {
        void* memory = OSReserve(reserve);
        if (memory && OSCommit(memory, MEM_INITIAL_COMMIT))
        {
            result = (Arena*)memory;
            *result = (Arena)
            {
                .curr = result,
                
                .alignment = alignment ? alignment : sizeof(iptr),
                .flags = ArenaFlag_NullTerminate|(growing ? ArenaFlag_Growing : 0),
                
                .commitPos = MEM_INITIAL_COMMIT,
                .cap = reserve,
            };
            
            u64 minPos = ArenaMinSize(result);
            result->pos = minPos;
            AsanPoison(result, reserve);
            AsanUnpoison(result, minPos);
        }
    }
    
    Assert(result);
    return result;
}

function void ArenaRelease(Arena* arena)
{
    for (Arena* ptr = arena->curr; ptr;)
    {
        Arena* prev = ptr->prev;
        AsanPoison(ptr, ptr->cap);
        OSRelease(ptr);
        ptr = prev;
    }
}

function void* ArenaPush(Arena* arena, u64 size)
{
    void* result = ArenaPushNZ(arena, size);
#if !BASE_ZERO_ON_POP
    ZeroMem(result, size);
#endif
    return result;
}

function void* ArenaPushNZ(Arena* arena, u64 size)
{
    void* result = 0;
    Arena* current = arena->curr;
    if (NEVER(current->alignment > ARCH_PAGE_SIZE))
        current->alignment = ARCH_PAGE_SIZE;
    u64 alignedPos = AlignUpPow2(current->pos, current->alignment);
    
    // allocate new chunk if necessary
    if (current->flags & ArenaFlag_Growing)
    {
        if (alignedPos + size > current->cap)
        {
            u64 minSize = ArenaMinSize(current);
            u64 newSize = AlignUpPow2(size + minSize, current->alignment);
            u64 newReserveSize = Max(current->cap, newSize);
            
            void* memory = OSReserve(newReserveSize);
            if (OSCommit(memory, MEM_INITIAL_COMMIT))
            {
                AsanPoison(memory, newReserveSize);
                AsanUnpoison(memory, minSize);
                Arena* new_chunk = (Arena*)memory;
                *new_chunk = (Arena)
                {
                    .prev = current,
                    .curr = new_chunk,
                    
                    .alignment = current->alignment,
                    .flags = current->flags,
                    
                    .basePos = current->basePos + current->cap,
                    .pos = minSize,
                    .commitPos = MEM_INITIAL_COMMIT,
                    .cap = newReserveSize,
                };
                
                current->curr = arena->curr = new_chunk;
                current = new_chunk;
                alignedPos = AlignUpPow2(current->pos, current->alignment);
            }
        }
    }
    
    u64 nextPos = alignedPos + size;
    if (nextPos <= current->cap)
    {
        if (nextPos > current->highWaterMark)
            current->highWaterMark = nextPos;
        
        if (nextPos > current->commitPos)
        {
            u64 nextCommitPos = ClampTop(AlignUpPow2(nextPos, MEM_COMMIT_BLOCK_SIZE), current->cap);
            u64 commitSize = nextCommitPos - current->commitPos;
            
            if (OSCommit(PtrAdd(current, current->commitPos), commitSize))
                current->commitPos = nextCommitPos;
        }
        
        if (nextPos <= current->commitPos)
        {
            result = PtrAdd(current, alignedPos);
            current->pos = nextPos;
            AsanUnpoison(result, size);
        }
    }
    
    Assert(result);
    return result;
}

function void ArenaPop(Arena* arena, u64 amount)
{
    u64 pos = ArenaPos(arena);
    u64 minSize = ArenaMinSize(arena);
    Assert(pos >= minSize);
    if (ALWAYS(pos - minSize > amount))
        ArenaPopTo(arena, pos - amount);
}

function void ArenaPopTo(Arena* arena, u64 pos)
{
    Arena* current = arena->curr;
    u64 currPos = current->basePos + current->pos;
    Assert(pos <= currPos);
    if (pos < currPos)
    {
        u64 clampedPos = ClampBot(pos, sizeof(Arena));
        u64 nextCommitPos = ClampTop(AlignUpPow2(clampedPos, MEM_COMMIT_BLOCK_SIZE), current->basePos + current->cap);
        
        while (nextCommitPos <= current->basePos)
        {
            Arena* prev = current->prev;
            u64 cap = current->cap;
            OSDecommit(current, cap); // @RECONSIDER(long): Maybe OSRelease?
            AsanPoison(current, cap); // @RECONSIDER(long): Do I need to poison the decommitted memory?
            current = prev;
        }
        
        arena->curr = current;
        nextCommitPos -= current->basePos;
        
        if (nextCommitPos < current->commitPos)
        {
            u8* nextPtr = PtrAdd(current, nextCommitPos);
            OSDecommit(nextPtr, current->commitPos - nextCommitPos);
            AsanPoison(nextPtr, current->commitPos - nextCommitPos);
            current->commitPos = nextCommitPos;
            current->pos = ClampTop(current->pos, current->commitPos);
        }
        else if (NEVER(nextCommitPos > current->commitPos))
        {
            // NOTE(long): Technically speaking, this can happen when:
            // 1. the user pushes a new chunk that surpasses the current cap
            // When this happens, a new arena will be allocated to contain the allocation, leaving the old arena untouched
            // This means the old arena could contain an uncommitted region from the commitPos to cap
            // Later, the user can pop into that padded region, and because the region is never allocated, it is poisonous
            // I can handle that case by actually allocating that region, but if this happens there's a bug somewhere
            // Unless I have an actual use case here, I'm asserting this for now
            
            // 2. MEM_COMMIT_BLOCK_SIZE <= MEM_INITIAL_COMMIT
            // I never find a good use case for this, so I statically check for it here
            StaticAssert(MEM_COMMIT_BLOCK_SIZE <= MEM_INITIAL_COMMIT, checkMemDefault);
            return;
        }
        
        u64 newPos = clampedPos - current->basePos;
        u8* ptr = PtrAdd(current, newPos);
        
        Assert(nextCommitPos <= current->commitPos);
        Assert(newPos <= current->commitPos);
        Assert(current->pos <= current->commitPos);
        Assert(current->commitPos <= current->cap);
        
#if BASE_ZERO_ON_POP
# if ENABLE_SANITIZER
        AsanUnpoison(ptr, current->pos - newPos); // unpoison padding causes by alignment
# else
        u64 alignedPos = AlignUpPow2(newPos, MEM_POISON_ALIGNMENT);
        if (current->commitPos != alignedPos)
        {
            u8* commitPtr = PtrAdd(current, alignedPos);
            OSDecommit(commitPtr, current->commitPos - alignedPos);
            OSCommit  (commitPtr, current->commitPos - alignedPos);
        }
# endif
        
        ZeroMem(ptr, current->pos - newPos);
#endif
        
        AsanPoison(ptr, current->commitPos - newPos);
        current->pos = newPos;
    }
}

internal void ArenaPoison(Arena* arena, u64 size)
{
    Arena* current = arena->curr;
    if (ALWAYS(current->pos - ArenaMinSize(current) >= size))
    {
#if ENABLE_SANITIZER
        AsanPoison(PtrAdd(current, current->pos - size), size);
#else
        OSDecommit(PtrAdd(current, current->pos - size), size);
#endif
    }
}

function void* ArenaPushPoisonEx(Arena* arena, u64 size, u64 preSize, u64 postSize)
{
    void* result = 0;
    Arena* current = arena->curr;
    u32 allocAlignment = current->alignment;
    
    if (preSize)
    {
        current->alignment = MEM_POISON_ALIGNMENT;
        preSize  = AlignUpPow2( preSize, MEM_POISON_ALIGNMENT);
        ArenaPushNZ(arena, preSize);
        ArenaPoison(arena, preSize);
    }
    
    if (size)
    {
        current->alignment = Max(allocAlignment, MEM_POISON_ALIGNMENT);
        u64 alignedSize = AlignUpPow2(size, MEM_POISON_ALIGNMENT);
        result = (u8*)ArenaPush(arena, alignedSize) + (alignedSize - size);
    }
    
    if (postSize)
    {
        current->alignment = MEM_POISON_ALIGNMENT;
        postSize = AlignUpPow2(postSize, MEM_POISON_ALIGNMENT);
        ArenaPushNZ(arena, postSize);
        ArenaPoison(arena, postSize);
    }
    
    current->alignment = allocAlignment;
    return result;
}

function void ArenaAlignNZ(Arena* arena, u32 alignment)
{
    Arena* current = arena->curr;
    u32 oldAlignment = current->alignment;
    current->alignment = alignment;
    ArenaPushNZ(current, 0);
    Assert(current == current->curr);
    current->alignment = oldAlignment;
}

function void ArenaAlign(Arena* arena, u32 alignment)
{
    Arena* current = arena->curr;
    u8* start = PtrAdd(current, current->pos);
    ArenaAlignNZ(current, alignment);
    u8* end = PtrAdd(current, current->pos);
    ZeroMem(start, end - start);
}

function TempArena TempBegin(Arena* arena)
{
    return (TempArena){ arena, ArenaPos(arena) };
}

function void TempEnd(TempArena temp)
{
    ArenaPopTo(temp.arena, temp.pos);
}

#if !BASE_LIB_IMPORT_SYMBOLS && !BASE_LIB_RUNTIME_IMPORT
global threadvar Arena* scratchPool[SCRATCH_POOL_COUNT] = {0};

TempArena BASE_SHARABLE(GetScratch)(Arena** conflictArray, u64 count)
{
    Arena** pool = scratchPool;
    if (pool[0] == 0)
    {
        Arena** scratchSlot = pool;
        for (u64 i = 0; i < SCRATCH_POOL_COUNT; ++i, ++scratchSlot)
            *scratchSlot = ArenaMake(1);
    }
    
    TempArena result = {0};
    Arena** scratchSlot = pool;
    for (u64 i = 0; i < SCRATCH_POOL_COUNT; ++i, ++scratchSlot)
    {
        b32 noConflict = true;
        Arena** conflictPtr = conflictArray;
        for (u64 j = 0; j < count; ++j, ++conflictPtr)
        {
            if (*scratchSlot == *conflictPtr)
            {
                noConflict = false;
                break;
            }
        }
        
        if (noConflict)
        {
            result = TempBegin(*scratchSlot);
            break;
        }
    }
    
    return result;
}
#endif

//~ long: String Functions

//- long: Constructor Functions
function u8 ChrFromStr(String str, u64 idx)
{
    return idx < str.size ? str.str[idx] : 0;
}

function String StrFromCStr(u8* cstr)
{
    u8* ptr = cstr;
    for (; *ptr; ++ptr);
    return StrRange(cstr, ptr);
}

function String16 Str16FromWStr(u16* wstr)
{
    u16* ptr = wstr;
    for (; *ptr; ++ptr);
    return Str16(wstr, ptr - wstr);
}

function String StrChop(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return Str(str.str, str.size - clampedSize);
}

function String StrSkip(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return Str(str.str + clampedSize, str.size - clampedSize);
}

function String StrPrefix(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return Str(str.str, clampedSize);
}

function String StrPostfix(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return Str(str.str + str.size - clampedSize, clampedSize);
}

function String Substr(String str, u64 first, u64 opl)
{
    u64 clampedOpl = ClampTop(str.size, opl);
    u64 clampedFirst = ClampTop(clampedOpl, first);
    return Str(str.str + clampedFirst, clampedOpl - clampedFirst);
}

function String SubstrRange(String str, u64 first, u64 range)
{
    return Substr(str, first, first + range);
}

function StringJoin SubstrSplit(String str, String substr)
{
    StringJoin result = {0};
    if (StrIsSubstr(str, substr))
    {
        result.pre = StrRange(str.str, substr.str);
        result.mid = substr;
        result.post = StrRange(substr.str + substr.size, str.str + str.size);
    }
    return result;
}

function String StrTrim(String str, String arr, i32 dir)
{
    u64 first = 0;
    if (dir <= 0)
    {
        while (first < str.size)
        {
            if (ChrCompareArr(str.str[first], arr, 0))
                first++;
            else
                break;
        }
    }
    
    u64 end = str.size - 1;
    if (dir >= 0)
    {
        // NOTE(long): This can be > when first is actually initialized (dir == 0)
        while (end >= first)
        {
            if (ChrCompareArr(str.str[end], arr, 0))
                end--;
            else
                break;
        }
    }
    
    return end >= first ? Substr(str, first, end + 1) : ZeroStr;
}

//- long: Allocation Functions
function String StrCopy(Arena* arena, String str)
{
    String result = StrPush(arena, str.size);
    CopyMem(result.str, str.str, str.size);
    return result;
}

function String StrPush(Arena* arena, u64 size)
{
    b32 terminate = (arena->flags & ArenaFlag_NullTerminate) ? 1 : 0;
    String result = { PushArrayNZ(arena, u8, size + terminate), size };
    if (terminate)
        result.str[size] = 0;
    return result;
}

function String StrFromFlags(Arena* arena, String* names, u64 nameCount, u64 flags)
{
    String result = {0};
    StringList list = {0};
    
    ScratchBlock(scratch, arena)
    {
        for (u64 i = 0; i < nameCount; ++i)
            if (flags & (1ULL << i))
                StrListPush(scratch, &list, names[i]);
        result = StrJoin(arena, &list, .mid = StrLit(" | "));
    }
    
    return result;
}

function StringList StrList(Arena* arena, String* strArr, u64 count)
{
    StringList result = {0};
    if (count)
    {
        result.first = PushArrayNZ(arena, StringNode, count);
        result.last = result.first + count - 1;
        for (u64 i = 0; i < count; ++i)
            StrListPushNode(&result, strArr ? strArr[i] : ZeroStr, result.first + i);
    }
    return result;
}

function void StrListPushNode(StringList* list, String str, StringNode* nodeMem)
{
    nodeMem->string = str;
    SLLQueuePush(list->first, list->last, nodeMem);
    list->nodeCount++;
    list->totalSize += str.size;
}

function void StrListPush(Arena* arena, StringList* list, String str)
{
    StringNode* node = PushStruct(arena, StringNode);
    StrListPushNode(list, str, node);
}

function void StrListConcatIP(StringList* list, StringList* toPush)
{
    if (toPush->nodeCount)
    {
        if (list->last)
        {
            list->nodeCount += toPush->nodeCount;
            list->totalSize += toPush->totalSize;
            list->last->next = toPush->first;
            list->last = toPush->last;
        }
        else *list = *toPush;
    }
}

function String StrListPopFront(StringList* list)
{
    String result = {0};
    StringNode* node = list->first;
    SLLQueuePop(list->first, list->last);
    if (node)
    {
        result = node->string;
        list->nodeCount--;
        list->totalSize -= result.size;
    }
    return result;
}

function String StrListPop(StringList* list)
{
    String result = {0};
    StringNode* node = 0;
    StringNode* tail = 0;
    for (node = tail = list->first; tail && tail->next; node = tail, tail = tail->next);
    if (tail)
    {
        Assert(list->last == tail);
        result = tail->string;
        list->last = node == tail ? (list->first = 0) : (node->next = 0, node);
        list->nodeCount--;
        list->totalSize -= result.size;
    }
    return result;
}

// NOTE(long): I could remove stdarg but stb_sprintf has already included it so what's the point
#include <stdarg.h>

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
CLANG(WarnDisable("-Wdouble-promotion"))
#include "stb_sprintf.h"
CLANG(WarnEnable("-Wdouble-promotion"))

function String StrPushfv(Arena* arena, char* fmt, va_list args)
{
    // in case we need to try a second time
    va_list args2;
    va_copy(args2, args);
    
    // try to build the string in 1024 bytes
    u32 bufferSize = 1024;
    u8* buffer = PushArrayNZ(arena, u8, bufferSize);
    // NOTE(long): vsnprintf doens't count the null terminator
    i32 size = stbsp_vsnprintf((char*)buffer, bufferSize, fmt, args);
    u32 allocSize = size + 1;
    
    String result = {0};
    if (ALWAYS(size >= 0))
    {
        if (allocSize <= bufferSize)
        {
            // if first try worked, put back the remaining
            ArenaPop(arena, bufferSize - allocSize);
            result = Str(buffer, size);
        }
        else
        {
            // if first try failed, reset and try again with the correct size
            ArenaPop(arena, bufferSize);
            u8* newBuffer = PushArrayNZ(arena, u8, allocSize);
            size = stbsp_vsnprintf((char*)newBuffer, allocSize, fmt, args2);
            if (ALWAYS(size >= 0))
                result = Str(newBuffer, size);
        }
        
        if (result.str)
            result.str[result.size] = 0;
    }
    
    va_end(args2);
    return result;
}

CHECK_PRINTF_FUNC(2) function String StrPushf(Arena* arena, CHECK_PRINTF char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String result = StrPushfv(arena, (char*)fmt, args);
    va_end(args);
    return result;
}

function String StrPad(Arena* arena, String str, char chr, u32 count, i32 dir)
{
    u64 size = (dir == 0 ? count * 2 : count) + str.size;
    String result = str;
    if (size > str.size)
    {
        result = StrPush(arena, size);
        if (dir <= 0) SetMem(result.str, chr, count);
        if (dir >= 0) SetMem(result.str + size - count, chr, count);
    }
    return result;
}

function String StrInsert(Arena* arena, String str, u64 index, String value)
{
    index = ClampTop(index, str.size - 1);
    String result = StrPush(arena, str.size + value.size);
    CopyMem(result.str, str.str, index);
    CopyMem(result.str + index, value.str, value.size);
    CopyMem(result.str + index + value.size, str.str + index, value.size - index);
    return result;
}

function String StrRemove(Arena* arena, String str, u64 index, u64 count)
{
    index = ClampTop(index, str.size - 1);
    count = ClampTop(count, str.size - index - 1);
    if (index + count == str.size)
        return StrChop(str, count); // @RECONSIDER(long): Maybe just alloc a new null-terminated string
    
    String result = StrPush(arena, count);
    CopyMem(result.str, str.str, index);
    CopyMem(result.str + index, str.str + index + count, count);
    return result;
}

function String StrRepeat(Arena* arena, String str, u64 count)
{
    String result = {0};
    u64 size = count * str.size;
    if (size)
    {
        result = StrPush(arena, size);
        for (u64 i = 0; i < count; ++i)
            CopyMem(result.str + i * str.size, str.str, str.size);
    }
    return result;
}

function String ChrRepeat(Arena* arena, char chr, u64 count)
{
    String result = {0};
    if (count)
    {
        result = StrPush(arena, count);
        SetMem(result.str, chr, count);
    }
    return result;
}

#define StrJoinSize(join, count, totalSize) ((join)->pre.size + (join)->post.size + (join)->mid.size*((count)-1) + (totalSize))

function String StrJoinList(Arena* arena, StringList* list, StringJoin* join)
{
    NilPtr(StringJoin, join);
    String pre = join->pre, mid = join->mid, post = join->post;
    
    u64 size = StrJoinSize(join, ClampBot(list->nodeCount, 1), list->totalSize);
    String result = StrPush(arena, size);
    u8* ptr = result.str;
    
    CopyMem(ptr, pre.str, pre.size);
    ptr += pre.size;
    
    b32 isMid = false;
    StrListIter(list, node)
    {
        if (isMid)
        {
            CopyMem(ptr, mid.str, mid.size);
            ptr += mid.size;
        }
        
        CopyMem(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        isMid = true;
    }
    
    CopyMem(ptr, post.str, post.size);
    ptr += post.size;
    
    Assert(ptr == (result.str + result.size));
    return result;
}

function String StrJoinMax3(Arena* arena, StringJoin* join)
{
    StringList list = {0};
    StringNode pre, mid, post;
    StrListPushNode(&list, join->pre, &pre);
    StrListPushNode(&list, join->mid, &mid);
    StrListPushNode(&list, join->post, &post);
    return StrJoin(arena, &list);
}

function String StrJoinListArr(Arena* arena, StringList* lists, u64 count, StringJoin* perNode, StringJoin* perList)
{
    NilPtr(StringJoin, perList);
    NilPtr(StringJoin, perNode);
    
    u64 size = StrJoinSize(perList, count, 0);
    for (u64 i = 0; i < count; ++i)
        size += StrJoinSize(perNode, ClampBot(lists[i].nodeCount, 1), lists[i].totalSize);
    
    Arena* buffer = BufferFromArena(arena, size);
    u8* str = ArenaPtr(buffer);
    
    StrCopy(buffer, perList->pre);
    for (u64 i = 0; i < count; ++i)
    {
        if (i)
            StrCopy(buffer, perList->mid);
        StrJoinList(buffer, lists + i, perNode);
    }
    StrCopy(buffer, perList->post);
    
    return StrRange(str, ArenaPtr(buffer));
}

function StringList StrSplitArr(Arena* arena, String str, String splits, StringMatchFlags flags)
{
    StringList result = {0};
    
    if (splits.size && str.size)
    {
        b32 allowEmpty = flags & SplitStr_KeepEmpties;
        u8* ptr = str.str;
        u8* firstWord = ptr;
        u8* opl = str.str + str.size;
        
        // NOTE(long): < rather than <= because firstWord = ptr + 1 can crash at the end of buffer
        for (; ptr < opl; ++ptr)
        {
            if (ChrCompareArr(*ptr, splits, flags))
            {
                // NOTE(long): try to emit word, < rather than <= because we don't allow empty members
                // EX: split "A,B,,C" with "," -> { "A", "B", "C" }
                if ((firstWord < ptr) || (firstWord == ptr && allowEmpty))
                    StrListPush(arena, &result, StrRange(firstWord, ptr));
                firstWord = ptr + 1;
            }
        }
        
        // try to emit the final word
        if (firstWord < ptr || (firstWord == ptr && allowEmpty))
            StrListPush(arena, &result, StrRange(firstWord, ptr));
    }
    
    return result;
}

function StringList StrSplit(Arena* arena, String str, String split, StringMatchFlags flags)
{
    StringList result = {0};
    
    if (split.size && str.size)
    {
        b32 allowEmpty = flags & SplitStr_KeepEmpties;
        u8* ptr = str.str;
        u8* firstWord = ptr;
        u8* opl = str.str + str.size;
        
        // NOTE(long): < rather than <= because firstWord = ptr + 1 can crash at the end of buffer
        for (; ptr < opl; ++ptr)
        {
            String substr = StrRange(ptr, opl);
            if (StrStartsWith(substr, split, flags))
            {
                // NOTE(long): try to emit word, <= rather than < will allow empty members
                // EX: split "A,B,,C" with "," -> { "A", "B", "", "C" } vs { "A", "B", "C" }
                // EX: split "A,B," -> { "A", "B", "" } vs { "A", "B" }
                if (firstWord < ptr || (firstWord == ptr && allowEmpty))
                    StrListPush(arena, &result, StrRange(firstWord, ptr));
                ptr += split.size - 1;
                firstWord = ptr + 1;
            }
        }
        
        // try to emit the final word
        if (firstWord < ptr || (firstWord == ptr && allowEmpty))
            StrListPush(arena, &result, StrRange(firstWord, ptr));
    }
    
    return result;
}

function String StrReplaceArr(Arena* arena, String str, String charArr, String newStr, StringMatchFlags flags)
{
    StringList list = StrSplitArr(arena, str, charArr, SplitStr_KeepEmpties|flags);
    return StrJoin(arena, &list, .mid = newStr);
}

function String StrReplace(Arena* arena, String str, String oldStr, String newStr, StringMatchFlags flags)
{
    StringList list = StrSplit(arena, str, oldStr, SplitStr_KeepEmpties|flags);
    return StrJoin(arena, &list, .mid = newStr);
}

function String StrToLower(Arena* arena, String str)
{
    String result = StrPush(arena, str.size);
    for (u64 i = 0; i < str.size; ++i)
        result.str[i] = IsCharacter(str.str[i]) ? ChrToLower(str.str[i]) : str.str[i];
    return result;
}

function String StrToUpper(Arena* arena, String str)
{
    String result = StrPush(arena, str.size);
    for (u64 i = 0; i < str.size; ++i)
        result.str[i] = IsCharacter(str.str[i]) ? ChrToUpper(str.str[i]) : str.str[i];
    return result;
}

function void StrToLowerIP(String str)
{
    for (u64 i = 0; i < str.size; ++i)
        str.str[i] = IsCharacter(str.str[i]) ? ChrToLower(str.str[i]) : str.str[i];
}

function void StrToUpperIP(String str)
{
    for (u64 i = 0; i < str.size; ++i)
        str.str[i] = IsCharacter(str.str[i]) ? ChrToUpper(str.str[i]) : str.str[i];
}

//- long: Comparision Functions
function b32 ChrCompare(char a, char b, StringMatchFlags flags)
{
    if ((flags & MatchStr_IgnoreCase) && IsCharacter(a) && IsCharacter(b))
        return ChrCompareNoCase(a, b);
    if ((flags & MatchStr_IgnoreSlash) && IsSlash(a))
        return IsSlash(b);
    return a == b;
}

function b32 StrCompare(String a, String b, StringMatchFlags flags)
{
    b32 result = a.size == b.size || (flags & MatchStr_RightSloppy);
    if (result)
    {
        if (a.str != b.str)
        {
            u64 size = Min(a.size, b.size);
            for (u64 i = 0; i < size && result; ++i)
                if (!ChrCompare(a.str[i], b.str[i], flags))
                    result = 0;
        }
    }
    
    return result;
}

function b32 ChrCompareArr(char chr, String arr, StringMatchFlags flags)
{
    for (u64 i = 0; i < arr.size; ++i)
        if (ChrCompare(chr, arr.str[i], flags))
            return true;
    return false;
}

function b32 StrCompareListEx(String str, StringList* values, StringMatchFlags flags, StringNode** outNode, u64* outIdx)
{
    b32 result = 0;
    NilPtr(StringNode*, outNode);
    NilPtr(u64, outIdx);
    
    u64 idx = 0;
    StrListIter(values, node)
    {
        if (StrCompare(str, node->string, flags))
        {
            *outNode = node;
            *outIdx  = idx;
            result = 1;
            break;
        }
        ++idx;
    }
    
    return result;
}

function b32 StrIsWhitespace(String str)
{
    for (u64 i = 0; i < str.size; ++i)
        if (!IsWspace(str.str[i]))
            return false;
    return true;
}

function i64 StrFindStr(String str, String val, StringMatchFlags flags)
{
    for (i64 i = 0; i < (i64)(str.size - val.size) + 1; ++i)
    {
        i64 index = (flags & MatchStr_LastMatch) ? (str.size - i - 1) : i;
        if (StrCompare(Str(str.str + index, val.size), val, flags))
            return index;
    }
    return -1;
}

function i64 StrFindArr(String str, String arr, StringMatchFlags flags)
{
    for (u64 i = 0; i < str.size; ++i)
    {
        i64 result = (flags & MatchStr_LastMatch) ? (str.size - i - 1) : i;
        if (ChrCompareArr(str.str[result], arr, flags))
            return result;
    }
    return -1;
}

function String StrChopAfter(String str, String arr, StringMatchFlags flags)
{
    i64 index = StrFindArr(str, arr, flags);
    return StrPrefix(str, index < 0 ? str.size : index);
}

function String StrSkipUntil(String str, String arr, StringMatchFlags flags)
{
    i64 index = StrFindArr(str, arr, flags);
    return StrSkip(str, index + 1);
}

//- long: Path Helpers

function PathStyle PathStyleFromStr(String str)
{
    PathStyle result = PathStyle_Relative;
    if (StrCompare(str, StrLit("/"), MatchStr_RightSloppy))
        result = PathStyle_UnixAbsolute;
    else if ((str.size >= 2 && IsCharacter(str.str[0]) && str.str[1] == ':') &&
             (str.size == 2 || IsSlash(str.str[2])))
        result = PathStyle_WindowsAbsolute;
    return result;
}

function String PathRelFromAbs(Arena* arena, String dst, String src)
{
    ScratchBegin(scratch, arena);
    StringList srcFolders = StrSplitPath(scratch, StrChopLastSlash(src));
    StringList dstFolders = StrSplitPath(scratch, StrChopLastSlash(dst));
    
    // long: count the number of backtracks to get from src -> dest
    StringNode* uniqueNode = dstFolders.first;
    u64 backtrackCount = srcFolders.nodeCount;
    for (StringNode* srcNode = srcFolders.first; srcNode && uniqueNode; srcNode = srcNode->next, uniqueNode = uniqueNode->next)
    {
        if (StrCompare(srcNode->string, uniqueNode->string, OSPathMatchFlags))
            backtrackCount -= 1;
        else
            break;
    }
    
    // long: If getting to `dst` from `src` requires erasing the entire `src`
    // Then the result should be the absolute path
    String result = {0};
    if (backtrackCount >= srcFolders.nodeCount)
        result = PathNormString(arena, dst);
    else
    {
        StringList dst_path_strs = {0};
        for (u64 i = 0; i < backtrackCount; ++i)
            StrListPush(scratch, &dst_path_strs, StrLit(".."));
        
        for (; uniqueNode; uniqueNode = uniqueNode->next)
            StrListPush(scratch, &dst_path_strs, uniqueNode->string);
        
        StrListPush(scratch, &dst_path_strs, StrSkipLastSlash(dst));
        result = StrJoin(arena, &dst_path_strs, .mid = StrLit("/"));
    }
    
    ScratchEnd(scratch);
    return result;
}

function String PathAbsFromRel(Arena* arena, String dst, String src)
{
    String result = dst;
    PathStyle style = PathStyleFromStr(dst);
    if (style == PathStyle_Relative)
    {
        ScratchBegin(scratch, arena);
        result = StrPushf(scratch, "%.*s/%.*s", StrExpand(src), StrExpand(dst));
        result = PathNormString(arena, result);
        ScratchEnd(scratch);
    }
    return result;
}

function String PathNormString(Arena* arena, String path)
{
    ScratchBegin(scratch, arena);
    PathStyle style = PathStyle_Relative;
    StringList list = PathListNormString(scratch, path, &style);
    String result = StrJoinPaths(arena, &list, style);
    ScratchEnd(scratch);
    return result;
}

function StringList PathListNormString(Arena* arena, String path, PathStyle* out)
{
    PathStyle pathStyle = PathStyleFromStr(path);
    StringList pathList = StrSplitPath(arena, path);
    
    if (pathList.nodeCount && pathStyle == PathStyle_Relative)
    {
        String pathStr = OSGetCurrDir(arena);
        pathStyle = PathStyleFromStr(pathStr);
        Assert(pathStyle != PathStyle_Relative);
        
        StringList currentPath = StrSplitPath(arena, pathStr);
        StrListConcatIP(&currentPath, &pathList);
        pathList = currentPath;
    }
    
    PathListResolveDotsIP(&pathList, pathStyle);
    
    if (out)
        *out = pathStyle;
    return(pathList);
}

function void PathListResolveDotsIP(StringList* path, PathStyle style)
{
    ScratchBegin(scratch);
    
    StringMetaNode* stack = 0;
    StringMetaNode* freeNode = 0;
    StringNode* first = path->first;
    
    ZeroStruct(path);
    for (StringNode* node = first,* next = 0; node; node = next)
    {
        // save next now
        next = node->next;
        
        // cases:
        if (node == first && style == PathStyle_WindowsAbsolute)
            goto SAVE_WITHOUT_STACK;
        if (StrCompare(node->string, StrLit( "."), 0))
            goto NOOP;
        if (StrCompare(node->string, StrLit(".."), 0))
        {
            if (stack)
                goto POP_STACK;
            else
                goto SAVE_WITHOUT_STACK;
        }
        goto SAVE_WITH_STACK;
        
        // handlers:
        SAVE_WITH_STACK:
        {
            StrListPushNodeMem(path, node);
            StringMetaNode* stackNode = freeNode;
            if (stackNode)
                SLLStackPop(freeNode);
            else
                stackNode = PushArrayNZ(scratch, StringMetaNode, 1);
            
            SLLStackPush(stack, stackNode);
            stackNode->node = node;
            continue;
        }
        
        SAVE_WITHOUT_STACK:
        {
            StrListPushNodeMem(path, node);
            continue;
        }
        
        POP_STACK:
        {
            path->nodeCount -= 1;
            path->totalSize -= stack->node->string.size;
            freeNode = stack;
            SLLStackPop(stack);
            
            if (!stack)
                path->last = path->first;
            else
                path->last = stack->node;
            continue;
        }
        
        NOOP: continue;
    }
    
    ScratchEnd(scratch);
}

//- long: Mutable Functions
function String StrWriteToStr(String src, u64 srcOffset, String dst, u64 dstOffset)
{
    String result = {0};
    if (srcOffset + src.size <= dstOffset + dst.size)
    {
        u8* ptr = dst.str + dstOffset;
        CopyMem(ptr, src.str + srcOffset, src.size);
        result = Str(ptr, src.size);
    }
    return result;
}

//- long: Unicode Functions
// NOTE(long): The standard now recommends replacing each error with the replacement character
// https://en.wikipedia.org/wiki/UTF-8#Error_handling
#define InvalidRune 0xFFFD
#define InvalidDecoder (StringDecode){ InvalidRune, 1 }

function StringDecode StrDecodeUTF8(u8* str, u64 cap)
{
    local const u8 length[] = {
        1, 1, 1, 1, // 0xxxx
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        0, 0, 0, 0, // 10xxx
        0, 0, 0, 0,
        2, 2, 2, 2, // 110xx
        3, 3,       // 1110x
        4,          // 11110
        0,          // 11111
    };
    local const u8 firstByteMask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
    local const u8 finalShift[] = { 0, 18, 12, 6, 0 };
    
    StringDecode result = {0};
    if (cap > 0)
    {
        result = InvalidDecoder;
        u8 byte = str[0];
        u8 l = length[byte >> 3];
        
        if (l == 0) result.error = DecodeError_InvalidBits;
        else if (l > cap) result.error = DecodeError_EOF;
        
        else
        {
            u32 cp = (byte & firstByteMask[l]) << 18;
            switch (l)
            {
                case 4: cp |= ((str[3] & 0x3F) <<  0);
                case 3: cp |= ((str[2] & 0x3F) <<  6);
                case 2: cp |= ((str[1] & 0x3F) << 12);
                default: break;
            }
            cp >>= finalShift[l];
            result = (StringDecode){cp, l};
            
            // NOTE(long): Accumulate the various error conditions
            local const u32 mins[] = { 4194304, 0, 128, 2048, 65536 };
            result.error |= (cp < mins[l]) * DecodeError_Overlong; // non-canonical encoding
            result.error |= ((cp >> 11) == 0x1b) * DecodeError_Surrogate;
            result.error |= (cp > 0x10FFFF) * DecodeError_OutOfRange;
            
            u8 cbits = 0; // top two bits of each tail byte
            switch (l)
            {
                case 4: cbits |= ((str[3] & 0xC0) >> 2);
                case 3: cbits |= ((str[2] & 0xC0) >> 4);
                case 2: cbits |= ((str[1] & 0xC0) >> 6);
                default: break;
            }
            
            result.error |= (cbits == 0x2A) * DecodeError_InvalidBits;
        }
    }
    else result.error = DecodeError_EOF;
    
    return result;
}

function StringDecode StrDecodeWide(u16* str, u64 cap)
{
    StringDecode result = InvalidDecoder;
    
    u16 x = str[0];
    if (x < 0xD800 || 0xDFFF < x) result.codepoint = x;
    else if (cap >= 2)
    {
        u16 y = str[1];
        if (0xD800 <= x && x < 0xDC00 &&
            0xDC00 <= y && y < 0xE000)
        {
            u16 xj = x - 0xD800;
            u16 yj = y - 0xDc00;
            u32 xy = (xj << 10) | yj;
            result = (StringDecode){ xy + 0x10000, 2 };
        }
    }
    
    return result;
}

function u32 StrEncodeUTF8(u8* dst, u32 codepoint)
{
    u32 size;
    if (codepoint < (1 << 7))
    {
        dst[0] = (u8)codepoint;
        size = 1;
    }
    
    else if (codepoint < (1 << 11))
    {
        dst[0] = 0xC0 | (u8)(codepoint >> 6);
        dst[1] = 0x80 | (u8)(codepoint & 0x3F);
        size = 2;
    }
    
    else if (codepoint < (1 << 16))
    {
        CP16:
        dst[0] = 0xE0 | (u8)(codepoint >> 12);
        dst[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[2] = 0x80 | (codepoint & 0x3F);
        size = 3;
    }
    
    else if (codepoint < 0x10FFFF)
    {
        dst[0] = 0xF0 | (u8)(codepoint >> 18);
        dst[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        dst[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[3] = 0x80 | (codepoint & 0x3F);
        size = 4;
    }
    
    else
    {
        StaticAssert(InvalidRune < 0x10FFFF);
        codepoint = InvalidRune;
        goto CP16;
    }
    return size;
}

function u32 StrEncodeWide(u16* dst, u32 codepoint)
{
    u32 size;
    if (codepoint < 0x10000)
    {
        BMP: // Basic Multilingual Plane
        dst[0] = (u16)codepoint;
        size = 1;
    }
    
    else if (codepoint < 0x10FFFF)
    {
        u32 cpj = codepoint - 0x10000;
        dst[0] = (u16)(cpj >>   10) + 0xD800;
        dst[1] = (cpj & 0x3FF) + 0xDC00;
        size = 2;
    }
    
    else
    {
        StaticAssert(InvalidRune < 0x10FFFF);
        codepoint = InvalidRune;
        goto BMP;
    }
    return size;
}

#undef InvalidRune
#undef InvalidDecoder

function String32 Str32FromStr(Arena* arena, String str)
{
    u64 maxSize = str.size;
    if (!maxSize) return ZeroStr32;
    
    u32* memory = PushArrayNZ(arena, u32, maxSize + 1);
    u32* dptr = memory;
    Str8Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (decode.error)
            return (String32){0};
        
        *dptr = decode.codepoint;
        ptr += decode.size;
        dptr++;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPop(arena, (maxSize - size) * sizeof(*memory));
    return Str32(memory, size);
}

function String16 Str16FromStr(Arena* arena, String str)
{
    u64 maxSize = str.size;
    if (!maxSize) return ZeroStr16;
    
    u16* memory = PushArrayNZ(arena, u16, maxSize + 1);
    u16* dptr = memory;
    Str8Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (decode.error)
            return (String16){0};
        
        u32 encSize = StrEncodeWide(dptr, decode.codepoint);
        ptr += decode.size;
        dptr += encSize;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPop(arena, (maxSize - size) * sizeof(*memory));
    return Str16(memory, size);
}

function String StrFromStr32(Arena* arena, String32 str)
{
    u64 maxSize = str.size*4;
    u8* memory = PushArrayNZ(arena, u8, maxSize + 1);
    
    u8* dptr = memory;
    Str32Stream(str, ptr, opl)
    {
        u32 encSize = StrEncodeUTF8(dptr, *ptr);
        ptr++;
        dptr += encSize;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPop(arena, (maxSize - size) * sizeof(*memory));
    return Str(memory, size);
}

function String StrFromStr16(Arena* arena, String16 str)
{
    // NOTE(long): UTF-16 starts to encode with 2 units at the same time UTF8 starts to encode with 4 (0x01000-0x10FFFF)
    // This means that a single unit in UTF-16 can be converted to at most three units in UTF-8
    u64 maxSize = str.size*3;
    u8* memory = PushArrayNZ(arena, u8, maxSize + 1);
    
    u8* dptr = memory;
    Str16Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeWide(ptr, (u64)(opl - ptr));
        u32 encSize = StrEncodeUTF8(dptr, decode.codepoint);
        ptr += decode.size;
        dptr += encSize;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPop(arena, (maxSize - size) * sizeof(*memory));
    return Str(memory, size);
}

function String UTF8Delete(String str, u64 count)
{
    if (str.size)
    {
        u64 i = str.size - 1;
        for (u64 runeLeft = count; i > 0 && runeLeft > 0; --i)
            if (str.str[i] <= 0x7F || str.str[i] >= 0xC0)
                runeLeft--;
        str.size = i;
    }
    return str;
}

function u64 UTF8Length(String str)
{
    u64 result = 0;
    Str8Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (decode.error)
            return 0;
        
        ptr += decode.size;
        result++;
    }
    return result;
}

function u32 UTF8GetErr(String str, u64* firstErrIdx)
{
    StringDecode decode = {0};
    NilPtr(u64, firstErrIdx);
    
    Str8Stream(str, ptr, opl)
    {
        decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (decode.error)
        {
            *firstErrIdx = ptr - str.str;
            break;
        }
        
        ptr += decode.size;
    }
    return decode.error;
}

//- long: Convert Functions
f32 FromCharsF32(const char* first, const char* last, b32* error);
f64 FromCharsF64(const char* first, const char* last, b32* error);

function f32 F32FromStr(String str, b32* error)
{
    f32 result = FromCharsF32(str.str, str.str + str.size, error);
    return result;
}

function f64 F64FromStr(String str, b32* error)
{
    f64 result = FromCharsF64(str.str, str.str + str.size, error);
    return result;
}

function u32 U32FromStr(String str, u32 radix, b32* error)
{
    NilB32(error);
    u64 result = U64FromStr(str, radix, error);
    if (result > MAX_U32)
        *error = 1;
    return (u32)result; // @UB(long): Implementation define?
}

function u64 U64FromStr(String str, u32 radix, b32* error)
{
    const u8 symbols[256] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0x0A,0xFB,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    };
    
    NilB32(error);
    u64 x = 0;
    
    if (str.size)
    {
        if (ALWAYS(radix >= 2 && radix <= 16))
        {
            for (u64 i = 0; i < str.size; ++i)
            {
                u8 symbol = symbols[str.str[i]];
                if (symbol >= radix)
                {
                    *error = 1;
                    goto END;
                }
                
                x = x * radix + symbol;
            }
        }
        *error = 0;
    }
    
    END:
    return *error ? 0 : x;
}

function i32 I32FromStr(String str, u32 radix, b32* error)
{
    NilB32(error);
    i64 result = I64FromStr(str, radix, error);
    if (result > MAX_I32 || result < MIN_I32)
        *error = 1;
    return (i32)result; // @UB(long): Implementation define?
}

function i64 I64FromStr(String str, u32 radix, b32* error)
{
    NilB32(error);
    i64 result = 0;
    
    if (str.size)
    {
        b64 negative = (str.str[0] == '-') * -1;
        if (negative || str.str[0] == '+')
            str = StrSkip(str, 1);
        u64 x = U64FromStr(str, radix, error);
        if (x > MAX_I64)
            *error = 1;
        else
            result = ((i64)x ^ negative) - negative;
    }
    
    return result;
}

function String StrFromTime(Arena* arena, DateTime time)
{
    b32 am = time.hour < 12;
    u8 hour = am ? (time.hour > 0 ? time.hour : 12) : (time.hour > 12 ? time.hour - 12 : 12);
    return StrPushf(arena, "%02u/%02u/%04u %02u:%02u %s", time.day, time.mon, (u16)time.year, hour, time.min, am ? "AM" : "PM");
}

function String StrFromF32(Arena* arena, f32 x, u32 prec)
{
    return StrFromF64(arena, (f64)x, prec);
}

function String StrFromF64(Arena* arena, f64 x, u32 prec)
{
    String result = StrPushf(arena, "%.*g", (i32)prec + 1, x);
    return result;
}

function String StrFromI32(Arena* arena, i32 x, u32 radix)
{
    return StrFromI64(arena, (i64)x, radix);
}

function String StrFromI64(Arena* arena, i64 x, u32 radix)
{
    const u8 symbols[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F', };
    String result = {0};
    
    if (radix >= 2 && radix <= 16)
    {
        if (x)
        {
            u8 space[64];
            u64 length = 0;
            
            u64 offset = 0;
            if (x < 0)
            {
                x = -x;
                space[offset++] = '-';
            }
            
            for (u64 i = x; i > 0; i /= radix, ++length)
                space[length+offset] = symbols[i%radix];
            
            for (u64 j = 0, i = length - 1; j < i; ++j, i -= 1)
                Swap(u8, space[i+offset], space[j+offset]);
            
            result = StrCopy(arena, Str(space, length + offset));
        }
        else
            result = StrCopy(arena, StrLit("0"));
    }
    return result;
}

//~ long: CLI Parsing

function CmdLine CmdLineFromList(Arena* arena, StringList* args)
{
    // NOTE(long): If the user types: myprogram.exe --foo -- a- // \' "abc, cde" /, \:a,b"d::"a
    // Then args will be: `myprogram.exe`, `--foo`, `--`, `a-`, `//`, `\'`, `abc, cde`, `\:a,bd::a`
    // AFAICT, there will never be any double quote inside any argument (`""` will become an empty string)
    // If the user types "myprogram" rather than "myprogram.exe" then the first arg will become "myprogram"
    
    CmdLine result = {0};
    result.programName = args->first->string;
    
    b32 passthrough = 0, isOptArg = 0;
    for (StringNode* node = args->first->next; node != 0; node = node->next)
    {
        Assert(!(passthrough && isOptArg));
        String str = node->string;
        // NOTE(long): If the user types in "", the OS will turn it into an empty string.
        // I just skip it for now, but this can be trivially handled if needed.
        if (str.size == 0)
            continue;
        
        b32 isOption = !passthrough && str.size > 0;
        if (isOption)
        {
            u8 c = str.str[0];
            
            // NOTE(long): "myprogram.exe abc --foo -bar def /baz 123 456"
            // `--foo`, `-bar`, and `/baz` are options while `abc` is an input
            // `def` is an argument for option `-bar`, while `123` and `456` are arguments for option `/baz`
            // All arguments after a single "--" (with no trailing string) will be considered as inputs.
            // So "myprogram.exe --a /b foo -- abc --def" will have 2 options (a, b) and 2 inputs (abc, --def)
            // `foo` will be an argument for `/b`
            if (StrCompare(str, StrLit("--"), MatchStr_RightSloppy))
            {
                if (str.size == 2)
                {
                    passthrough = 1;
                    isOptArg = 0;
                    continue;
                }
                str = StrSkip(str, 2);
            }
            
            else if (c == '-' || c == '/')
                str = StrSkip(str, 1);
            else
                isOption = 0;
        }
        
        if (isOption)
        {
            isOptArg = 0;
            i64 argPos = StrFindArr(str, StrLit(":="), 0);
            String optName = StrPrefix(str, argPos);
            StringList optArgs = {0};
            
            if (argPos >= 0)
            {
                String argStr = StrSkip(str, argPos+1);
                isOptArg = argStr.size == 0 || argStr.str[argStr.size-1] == ',';
                StringList argList = StrSplit(arena, argStr, StrLit(","), 0);
                StrListConcatIP(&optArgs, &argList);
            }
            
            else if (StrCompare(StrPostfix(str, 2), StrLit("--"), 0))
            {
                StrListPush(arena, &optArgs, StrLit("--"));
                optName = StrChop(str, 2);
            }
            
            else if (StrCompare(StrPostfix(str, 1), StrLit("-"), 0))
            {
                StrListPush(arena, &optArgs, StrLit("-"));
                optName = StrChop(str, 1);
            }
            
            CmdLineOpt* option = CmdOptPush(arena, &result.opts, optName);
            option->values = optArgs;
        }
        
        else if (isOptArg)
        {
            StringList optArgs = StrSplit(arena, str, StrLit(","), 0);
            StrListConcatIP(&result.opts.last->values, &optArgs);
            isOptArg = str.str[str.size-1] == ',';
        }
        
        else StrListPush(arena, &result.inputs, str);
    }
    
    return result;
}

function CmdLineOpt* CmdOptPush(Arena* arena, CmdLineOptList* list, String name)
{
    CmdLineOpt* opt = PushStruct(arena, CmdLineOpt);
    opt->name = name;
    SLLQueuePush(list->first, list->last, opt);
    list->count++;
    return opt;
}

function CmdLineOpt* CmdOptLookup(CmdLine* cmd, String name, StringMatchFlags flags)
{
    for (CmdLineOpt* opt = cmd->opts.first; opt; opt = opt->next)
        if (StrCompare(opt->name, name, flags))
            return opt;
    return 0;
}

function b32 CmdHasOpt(CmdLine* cmd, String name)
{
    CmdLineOpt* opt = CmdOptLookup(cmd, name, 0);
    return opt != 0;
}

function b32 CmdHasArg(CmdLine* cmd, String name)
{
    CmdLineOpt* opt = CmdOptLookup(cmd, name, 0);
    return opt->values.nodeCount != 0;
}

//- long: C-Syntax Functions
function String StrCEscape(Arena* arena, String str)
{
    // TODO(long): This doesn't handle hex/octal/unicode escape sequences right now, just the simple stuff
    StringList strs = {0};
    String result = {0};
    u64 start = 0;
    
    ScratchBlock(scratch, arena)
    {
        for (u64 i = 0; i <= str.size; ++i)
        {
            // NOTE(long): Check for '\r` to convert \r\n to \n
            if (i == str.size || str.str[i] == '\\' || str.str[i] == '\r')
            {
                String substr = Substr(str, start, i);
                if (substr.size)
                    StrListPush(arena, &strs, substr);
                start = i+1;
            }
            
            if (i < str.size - 1 && str.str[i] == '\\')
            {
                u8 replaceByte = 0;
                
                switch (str.str[i+1])
                {
                    default: break;
                    case 'a': replaceByte = 0x07; break;
                    case 'b': replaceByte = 0x08; break;
                    case 'e': replaceByte = 0x1b; break;
                    case 'f': replaceByte = 0x0c; break;
                    case 'n': replaceByte = 0x0a; break;
                    case 'r': replaceByte = 0x0d; break;
                    case 't': replaceByte = 0x09; break;
                    case 'v': replaceByte = 0x0b; break;
                    case '\\':replaceByte = '\\'; break;
                    case '\'':replaceByte = '\''; break;
                    case '"': replaceByte = '"';  break;
                }
                
                String replaceStr = StrCopy(scratch, StrFromChr(replaceByte));
                StrListPush(scratch, &strs, replaceStr);
                ++i;
                ++start;
            }
        }
        
        result = StrJoin(arena, &strs);
    }
    
    return result;
}

function i64 I64FromStrC(String str, b32* error)
{
    NilB32(error);
    i64 result = 0;
    
    if (str.size)
    {
        b64 negative = (str.str[0] == '-') * -1;
        if (negative || str.str[0] == '+')
            str = StrSkip(str, 1);
        u64 x = U64FromStrC(str, error);
        if (x > MAX_I64)
            *error = 1;
        else
            result = ((i64)x ^ negative) - negative;
    }
    
    return result;
}

function u64 U64FromStrC(String str, b32* error)
{
    u64 result = 0;
    String numeric = str;
    u32 base = 10;
    
    if (str.size >= 2)
    {
        if (str.str[0] == '0')
        {
            switch (str.str[1])
            {
                case 'x': base = 16; numeric = StrSkip(str, 2); break;
                case 'b': base =  2; numeric = StrSkip(str, 2); break;
                default:  base =  8; numeric = StrSkip(str, 1); break;
            }
        }
    }
    
    result = U64FromStr(numeric, base, error);
    return result;
}

//~ long: Logs/Errors

#define PRINT_INTERNAL_BUFFER_SIZE KiB(4)

CHECK_PRINTF_FUNC(1) function void Outf(CHECK_PRINTF char* fmt, ...)
{
    u8 buffer[PRINT_INTERNAL_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    i32 size = stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    if (ALWAYS(size >= 0))
    {
        String msg = Str(buffer, size);
        u8* extra = 0;
        
        if (size > PRINT_INTERNAL_BUFFER_SIZE)
        {
            msg.size = PRINT_INTERNAL_BUFFER_SIZE;
            i32 allocSize = size + 1;
            extra = OSCommit(0, allocSize);
            
            if (ALWAYS(extra))
            {
                va_start(args, fmt);
                size = stbsp_vsnprintf(extra, allocSize, fmt, args);
                va_end(args);
                if (ALWAYS(size))
                    msg = Str(extra, size);
            }
        }
        
        ALWAYS(OSWriteConsole(OS_STD_OUT, msg));
        if (extra)
            OSDecommit(extra, size);
    }
}

CHECK_PRINTF_FUNC(1) function void Errf(CHECK_PRINTF char* fmt, ...)
{
    u8 buffer[PRINT_INTERNAL_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    i32 size = stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
    if (ALWAYS(size >= 0))
    {
        if (NEVER(size > PRINT_INTERNAL_BUFFER_SIZE))
            size = PRINT_INTERNAL_BUFFER_SIZE;
        ALWAYS(OSWriteConsole(OS_STD_ERR, Str(buffer, size)));
    }
    va_end(args);
}

//- NOTE(long): This was heavily inspired by Ryan, Allen, and rxi
// https://www.rfleury.com/p/the-easiest-way-to-handle-errors
// https://youtu.be/-XethY-QrR0?t=133
// https://github.com/rxi/log.c

readonly global const String LogType_names[] =
{
    StrConst("TRACE"),
    StrConst("DEBUG"),
    StrConst("INFO"),
    StrConst("WARN"),
    StrConst("ERROR"),
    StrConst("FATAL"),
};

typedef struct LOG_Node LOG_Node;
struct LOG_Node
{
    LOG_Node* next;
    Record record;
};

typedef struct LOG_List LOG_List;
struct LOG_List
{
    LOG_List* next;
    Arena* arena;
    
    LOG_Node* first;
    LOG_Node* last;
    u64 count;
    
    LogInfo info;
};

typedef struct LOG_Thread LOG_Thread;
struct LOG_Thread
{
    Arena* arena;
    LOG_List* stack;
};

#if !BASE_LIB_IMPORT_SYMBOLS && !BASE_LIB_RUNTIME_IMPORT
global threadvar LOG_Thread logThread = {0};

void BASE_SHARABLE(LogBeginEx)(LogInfo info)
{
    if (!logThread.arena)
        logThread.arena = ArenaReserve(KiB(64), 1, 1);
    LOG_List* list = PushStruct(logThread.arena, LOG_List);
    *list = (LOG_List){ .arena = logThread.arena->curr, .info = info };
    SLLStackPush(logThread.stack, list);
}

Logger BASE_SHARABLE(LogEnd)(Arena* arena)
{
    Logger result = {0};
    LOG_List* list = logThread.stack;
    if (list)
    {
        result.records = PushArrayNZ(arena, Record, list->count);
        result.info = list->info;
        
        for (LOG_Node* node = list->first; node; node = node->next)
        {
            node->record.log = StrCopy(arena, node->record.log);
            result.records[result.count++] = node->record;
        }
        
        SLLStackPop(logThread.stack);
        ArenaPopTo(list->arena, list->arena->curr->basePos + ((u8*)list - (u8*)list->arena));
    }
    return result;
}

LogInfo BASE_SHARABLE(LogGetInfo)(void)
{
    return logThread.stack ? logThread.stack->info : (LogInfo){0};
}

CHECK_PRINTF_FUNC(4) void BASE_SHARABLE(LogPushf)(i32 level, char* file, i32 line, CHECK_PRINTF char* fmt, ...)
{
    LOG_List* list = logThread.stack;
    if (NEVER(!InRange(level, 0, LogType_Count - 1)))
        level = Clamp(level, 0, LogType_Count - 1);
    
    if (list)
    {
        if (NEVER(!InRange(list->info.level, 0, LogType_Count - 1)))
            list->info.level = Clamp(list->info.level, 0, LogType_Count - 1);
        
        if (level >= list->info.level)
        {
            LOG_Node* node = PushStruct(logThread.arena, LOG_Node);
            SLLQueuePush(list->first, list->last, node);
            list->count++;
            
            node->record = (Record){ .file = file, .line = line, .level = level };
            DateTime date = OSNowUniTime();
            date = OSToLocTime(date);
            node->record.time = TimeToDense(date);
            
            va_list args;
            va_start(args, fmt);
            list->info.callback(logThread.arena, &node->record, (char*)fmt, args);
            va_end(args);
        }
    }
}

void BASE_SHARABLE(LogFmtStd)(Arena* arena, Record* record, char* fmt, va_list args)
{
    ScratchBlock(scratch, arena)
    {
        DateTime time = TimeToDate(record->time);
        String log = StrPushfv(scratch, fmt, args);
        String file = PathRelFromAbs(scratch, StrFromCStr((u8*)record->file), OSGetExeDir());
        String level = StrToUpper(scratch, GetEnumStr(LogType, record->level));
        
        record->log = StrPushf(arena, "%02u:%02u:%02u %-5s %.*s:%d: %s", time.hour, time.min, time.sec,
                               level.str, StrExpand(file), record->line, log.str);
    }
}

void BASE_SHARABLE(LogFmtANSIColor)(Arena* arena, Record* record, char* fmt, va_list args)
{
    ScratchBlock(scratch, arena)
    {
        DateTime time = TimeToDate(record->time);
        String log = StrPushfv(scratch, fmt, args);
        String file = PathRelFromAbs(scratch, StrFromCStr((u8*)record->file), OSGetExeDir());
        String level = StrToUpper(scratch, GetEnumStr(LogType, record->level));
        
        // https://ss64.com/nt/syntax-ansi.html
        if (ALWAYS(InRange(record->level, 0, LogType_Count - 1)))
        {
            local const char* colors[] = { "\x1b[36m", "\x1b[94m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[95m" };
            record->log = StrPushf(arena, "[%02u:%02u:%02u] %s%5s \x1b[90m%.*s:%d: \x1b[97m%s\x1b[0m",
                                   time.hour, time.min, time.sec,
                                   colors[record->level], level.str,
                                   StrExpand(file), record->line,
                                   log.str);
        }
    }
}
#endif

function StringList StrListFromLogger(Arena* arena, Logger* logger)
{
    StringList result = {0};
    for (u64 i = 0; i < logger->count; ++i)
        StrListPush(arena, &result, logger->records[i].log);
    return result;
}

//~ long: Buffer Functiosn

function String
BufferInterleave(Arena *arena, void **in,
                 u64 laneCount, u64 elementSize, u64 elementCount){
    // TODO(allen): look at disassembly for real speed work
    
    // setup buffer
    String result = PushBufferNZ(arena, laneCount*elementSize*elementCount);
    
    // fill loop
    u8 *out_ptr = result.str;
    u64 in_off = 0;
    for (u64 i = 0; i < elementCount; i += 1, in_off += elementSize){
        u8 **in_base_ptr = (u8**)in;
        for (u64 j = 0; j < laneCount; j += 1, in_base_ptr += 1){
            CopyMem(out_ptr, *in_base_ptr + in_off, elementSize);
            out_ptr += elementSize;
        }
    }
    
    return result;
}

function String*
BufferUninterleave(Arena *arena, void *in,
                   u64 laneCount, u64 elementSize, u64 elementCount){
    // TODO(allen): look at disassembly for real speed work
    
    // compute sizes
    u64 bytes_per_lane = elementSize*elementCount;
    
    // allocate outs
    String *result = PushArrayNZ(arena, String, laneCount);
    for (u64 i = 0; i < laneCount; i += 1)
        result[i] = PushBufferNZ(arena, bytes_per_lane);
    
    // fill loop
    u8 *in_ptr = (u8*)in;
    u64 out_off = 0;
    for (u64 i = 0; i < elementCount; i += 1, out_off += elementSize){
        String *out_buffer = result;
        for (u64 j = 0; j < laneCount; j += 1, out_buffer += 1){
            CopyMem(out_buffer->str + out_off, in_ptr, elementSize);
            in_ptr += elementSize;
        }
    }
    
    return result;
}

//~/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////        WIN32 IMPLEMENT         ////////////////
////////////////////////////////////////////////////////////////
//-/////////////////////////////////////////////////////////////

#if OS_WIN
// NOTE(long): 28301: annotations weren't found at the first declaration of a given function
// I have zero idea what this warning does but it keep reports something stupid in winnt.h
// winnt.h(3454) : warning C28301: No annotations for first declaration of '_mm_clflush'. See <no file>(0). 
MSVC(WarnDisable(28301))
#pragma push_macro("function")
#undef function
#include <Windows.h>
#include <Userenv.h>
#include <dwmapi.h>
MSVC(WarnEnable(28301))
#pragma pop_macro("function")

//~ long: Memory Functions

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

function void* OSCommit(void* ptr, u64 size)
{
    // NOTE(long): https://devblogs.microsoft.com/oldnewthing/20151008-00/?p=91411
    // MEM_COMMIT with a NULL ptr is unspecified, but Windows typically let it slide
    void* result = VirtualAlloc(ptr, size, MEM_COMMIT|(ptr ? 0 : MEM_RESERVE), PAGE_READWRITE);
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

//~ long: Atomic Functions

function void* SLLAtomicPop_(void*volatile* first, uptr nextOff)
{
    void* old,** next;
    do {
        old = *first;
        next = PtrAdd(old, nextOff);
    } while (old && AtomicCmpXchPtr(first, *next, old) != old);
    return old;
}

//~ long: Global Variables

global u64 win32TicksPerSecond = 0;
global StringList win32CmdLine = {0};
global HINSTANCE win32Instance = {0};
global SysInfo win32SysInfo = {0};

global Arena* win32PermArena = {0};
global String win32BinaryPath = {0};
global String win32UserPath = {0};
global String win32TempPath = {0};

//~ long: Setup

#if BASE_LIB_RUNTIME_IMPORT
#include "psapi.h"

#define RUNTIME_FUNCS(X) \
    X(GetScratch) \
    X(LogBeginEx) \
    X(LogEnd) \
    X(LogGetInfo) \
    X(LogPushf) \
    X(LogFmtStd) \
    X(LogFmtANSIColor)

BeforeMain(BaseInitRuntime)
{
    b32 success = 0;
    HANDLE process = GetCurrentProcess();
    HMODULE modules[1024];
    DWORD needed = 0;
    
    if (EnumProcessModules(process, modules, sizeof(modules), &needed))
    {
        for (i32 i = 0; i < (needed / sizeof(HMODULE)) && !success; i++ )
        {
            TCHAR moduleName[1024];
            DEBUG(moduleName, GetModuleFileNameEx(process, modules[i], moduleName, ArrayCount(moduleName)));
            
#define X(name) PrcCast(name, GetProcAddress(modules[i], Stringify(name)));
            RUNTIME_FUNCS(X)
#undef X
            
#define X(name) if (!name) continue;
            RUNTIME_FUNCS(X)
#undef X
#undef RUNTIME_FUNCS
            
            success = 1;
        }
    }
    
    Assert(success); // TODO(long): Maybe use GFXErrorBox
}
#endif

BeforeMain(BaseInit)
{
    //- Setup precision time
    {
        LARGE_INTEGER perfFreq = {0};
        if (QueryPerformanceFrequency(&perfFreq))
            win32TicksPerSecond = ((u64)perfFreq.HighPart << 32) | perfFreq.LowPart;
        timeBeginPeriod(1);
    }
    
    win32PermArena = ArenaMake();
    ScratchBegin(scratch);
    
    SYSTEM_INFO sysInfo = {0};
    GetNativeSystemInfo(&sysInfo);
    
    //- Processor Arch
    switch (sysInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64: win32SysInfo.arch = Arch_X64;   break;
        case PROCESSOR_ARCHITECTURE_INTEL: win32SysInfo.arch = Arch_X86;   break;
        case PROCESSOR_ARCHITECTURE_ARM:   win32SysInfo.arch = Arch_ARM;   break;
        case PROCESSOR_ARCHITECTURE_ARM64: win32SysInfo.arch = Arch_ARM64; break;
        
        case PROCESSOR_ARCHITECTURE_IA64:
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default: win32SysInfo.arch = Arch_None;  break;
    }
    
    //- TODO(long): IsProcessorFeaturePresent to setup isaMask
    
    //- Virtual Address
    win32SysInfo.pageSize = sysInfo.dwPageSize;
    win32SysInfo.allocGranularity = sysInfo.dwAllocationGranularity;
    win32SysInfo.addressRange.min = IntFromPtr(sysInfo.lpMinimumApplicationAddress);
    win32SysInfo.addressRange.max = IntFromPtr(sysInfo.lpMaximumApplicationAddress);
    
    //- Core/Thread Count
    {
        DWORD length = 0;
        GetLogicalProcessorInformationEx(RelationProcessorCore, 0, &length);
        void* buffer = ArenaPush(scratch, length);
        
        if (GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &length))
        {
            DWORD size;
            for (u8* ptr = buffer,* end = ptr + length; ptr < end; ptr += size)
            {
                SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info = (void*)ptr;
                size = info->Size;
                
                if (ALWAYS(info->Relationship == RelationProcessorCore))
                {
                    win32SysInfo.coreCount++;
                    // NOTE(long): You can also use GetActiveProcessorCount(ALL_PROCESSOR_GROUPS)
                    // https://devblogs.microsoft.com/oldnewthing/20200824-00/?p=104116
                    // Sadly, I still need to do all of this to get the core count
                    for (WORD i = 0; i < info->Processor.GroupCount; ++i)
                        win32SysInfo.threadCount += (u16)__popcnt64(info->Processor.GroupMask[i].Mask);
                }
            }
        }
    }
    
    //- Machine Name
    {
        u16 buffer[MAX_COMPUTERNAME_LENGTH + 1] = {0};
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        if (GetComputerNameW(buffer, &size))
            win32SysInfo.machineName = StrFromStr16(win32PermArena, Str16(buffer, size));
    }
    
    //- Setup binary path
    {
        DWORD cap = 2048;
        u16* buffer = 0;
        DWORD size = 0;
        for (u64 r = 0; r < 4; ++r, cap *= 4)
        {
            u16* tryBuffer = PushArrayNZ(scratch, u16, cap);
            DWORD trySize = GetModuleFileNameW(0, tryBuffer, cap);
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
        {
            String binaryPath = StrFromStr16(scratch, Str16(buffer, size));
            binaryPath = StrChopAfter(binaryPath, StrLit("/\\"), MatchStr_LastMatch);
            win32BinaryPath = StrCopy(win32PermArena, binaryPath);
        }
    }
    
    //- Setup user path
    {
        HANDLE token = GetCurrentProcessToken();
        DWORD cap = 2048;
        u16* buffer = PushArrayNZ(scratch, u16, cap);
        if (!GetUserProfileDirectoryW(token, buffer, &cap))
        {
            ScratchClear(scratch);
            buffer = PushArrayNZ(scratch, u16, cap);
            if (!GetUserProfileDirectoryW(token, buffer, &cap))
                buffer = 0;
        }
        
        if (ALWAYS(buffer))
        {
            // NOTE(long): the docs make it sound like we can only count on cap getting the size on failure
            // so we're just going to cstring this to be safe.
            String16 userPath = Str16FromWStr(buffer);
            win32UserPath = StrFromStr16(win32PermArena, userPath);
        }
    }
    
    //- Setup temp path
    {
        DWORD cap = 2048;
        u16* buffer = PushArrayNZ(scratch, u16, cap);
        DWORD size = GetTempPathW(cap, buffer);
        if (size >= cap)
        {
            ScratchClear(scratch);
            buffer = PushArrayNZ(scratch, u16, size);
            size = GetTempPathW(size, buffer);
        }
        
        // NOTE(long): size - 1 because this particular string function in the Win32 API
        // is different from the others and it includes the trailing backslash.
        // We want consistency, so the "- 1" removes it.
        if (ALWAYS(size > 1))
            win32TempPath = StrFromStr16(win32PermArena, Str16(buffer, size - 1));
    }
    
    ScratchEnd(scratch);
}

StaticAssert(MAX_U32 == INFINITE, w32InfiniteCheck);
#define W32TimeMS(ms) ((DWORD)ms)

#define W32CheckHandle(handle) ((handle) != 0 && (handle) != INVALID_HANDLE_VALUE)
#define W32FromHandle(osHandle) ((HANDLE)((osHandle).v[0]))
#define W32ToHandle(w32Handle) ((OS_Handle){ .v[0] = (u64)(w32Handle) })

function b32 OS_HandleIsValid(OS_Handle handle)
{
    HANDLE process = W32FromHandle(handle);
    return W32CheckHandle(process);
}

function StringList OSSetArgs(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i)
    {
        String arg = StrFromCStr(argv[i]);
        StrListPush(win32PermArena, &win32CmdLine, arg);
    }
    return win32CmdLine;
}

function StringList OSGetArgs(void)
{
    return win32CmdLine;
}

function void OSExit(u32 code)
{
    ExitProcess(code);
}

//~ long: Win32 Specialized Functions

function void W32WinMainInit(HINSTANCE hInstance,
                             HINSTANCE hPrevInstance,
                             LPSTR lpCmdLine,
                             int nShowCmd)
{
    win32Instance = hInstance;
    UNUSED(hPrevInstance);
    UNUSED(lpCmdLine);
    UNUSED(nShowCmd);
    
    OSSetArgs(__argc, __argv);
}

function HINSTANCE W32GetInstance(void)
{
    return win32Instance;
}

//~ long: Time

internal DateTime W32DateTimeFromSystemTime(SYSTEMTIME* time)
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

internal SYSTEMTIME W32SystemTimeFromDateTime(DateTime time)
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

internal DenseTime W32DenseTimeFromFileTime(FILETIME* fileTime)
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
        result = ticks * KB(1) / win32TicksPerSecond;
    }
    return result;
}

function u64 OSNowUS(void)
{
    u64 result = 0;
    LARGE_INTEGER perfCounter = {0};
    if (QueryPerformanceCounter(&perfCounter))
    {
        u64 ticks = ((u64)perfCounter.HighPart << 32) | perfCounter.LowPart;
        result = ticks * MB(1) / win32TicksPerSecond;
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

//~ long: Console Handling

internal String W32ReadFile(Arena* arena, HANDLE file, r1u64 rng)
{
    String result = {0};
    
    r1u64 clampedRng;
    {
        u64 maxSize = 0;
        GetFileSizeEx(file, (PLARGE_INTEGER)&maxSize); // TODO(long): Endianness
        clampedRng = R1U64(ClampTop(rng.min, maxSize), ClampTop(rng.max, maxSize));
        if (rng.min == 0 && rng.max == 0)
            clampedRng.max = maxSize;
    }
    
    u64 size = DimR1U64(clampedRng);
    if (size)
    {
        TempArena restorePoint = TempBegin(arena);
        String str = StrPush(arena, size);
        
        u8* beg = str.str,* ptr = beg,* opl = beg + size;
        b32 success = 1;
        for (DWORD actualRead = 0; ptr < opl && success; ptr += actualRead)
        {
            DWORD readAmount = (u32)ClampTop((u64)(opl - ptr), MAX_U32);
            u64 off = ptr - beg + clampedRng.min;
            OVERLAPPED overlapped = {
                .Offset     = (off&0x00000000FFFFFFFFULL),
                .OffsetHigh = (off&0xffffffff00000000ULL) >> 32,
            };
            
            // NOTE(long): ReadFile will always zero out actualRead
            success = ReadFile(file, ptr, readAmount, &actualRead, &overlapped);
            success = success && actualRead;
        }
        
        if (success)
            result = str;
        else
            TempEnd(restorePoint);
    }
    
    return result;
}

// NOTE(long): CRT seems to be using 4KB for fgets internal buffer size
// But `cmd.exe` allows up to 8KB - 1 characters, so I use that instead
// https://learn.microsoft.com/en-us/troubleshoot/windows-client/shell-experience/command-line-string-limitation
#define CONSOLE_INTERNAL_BUFFER_SIZE KiB(8)

internal HANDLE W32GetStdHandle(i32 handle)
{
    DWORD handles[] =
    {
        0,
        STD_INPUT_HANDLE,
        STD_OUTPUT_HANDLE,
        STD_ERROR_HANDLE,
    };
    if (NEVER(!InRange(handle, 0, 3)))
        handle = 0;
    
    HANDLE result = GetStdHandle(handles[handle]);
    return result;
}

function String OSReadConsole(Arena* arena, i32 handle)
{
    String result = {0};
    
    HANDLE file = W32GetStdHandle(handle);
    if (W32CheckHandle(file))
    {
        DWORD fileType = GetFileType(file);
        if (fileType == FILE_TYPE_DISK)
            result = W32ReadFile(arena, file, (r1u64){0});
        
        else
        {
            TempArena restorePoint = TempBegin(arena);
            DWORD bufferSize = CONSOLE_INTERNAL_BUFFER_SIZE;
            DWORD actualRead = 0;
            String str = StrPush(arena, bufferSize);
            
            if (fileType == FILE_TYPE_PIPE)
            {
                // TODO(long): https://handmade.network/forums/t/8916-how_to_read_from_and_write_to_the_console_in_win32#30197
            }
            
            if (ReadFile(file, str.str, (DWORD)str.size, &actualRead, 0))
            {
                if (actualRead)
                {
                    result = Str(str.str, actualRead);
                    
                    // NOTE(long): Stupid Microsoft with stupid \r\n
                    if (StrCompare(StrPostfix(result, 2), StrLit("\r\n"), 0))
                        result.size -= 2;
                    else
                        Assert(actualRead == str.size);
                    
                    ArenaPop(arena, str.size - result.size);
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
    
    HANDLE file = W32GetStdHandle(handle);
    if (W32CheckHandle(file))
    {
        DWORD writeAmount = ALWAYS(data.size <= MAX_I32) ? (DWORD)data.size : MAX_I32;
        DWORD actualWrite = 0;
        result = WriteFile(file, data.str, writeAmount, &actualWrite, 0);
        result = result && ALWAYS(data.size == actualWrite);
    }
    
    return result;
}

// https://learn.microsoft.com/en-us/windows/console/clearing-the-screen
function void OSClearConsole(i32 handle)
{
    HANDLE console = W32GetStdHandle(handle);
    if (W32CheckHandle(console))
        return;
    
    // Get the number of character cells in the current buffer.
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(console, &csbi))
        return;
    
    // Scroll the rectangle of the entire buffer.
    SMALL_RECT scrollRect = {
        .Left = 0,
        .Top = 0,
        .Right = csbi.dwSize.X,
        .Bottom = csbi.dwSize.Y,
    };
    
    // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
    COORD scrollTarget;
    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);
    
    // Fill with empty spaces with the buffer's default text attribute.
    CHAR_INFO fill;
    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = csbi.wAttributes;
    
    // Do the scroll
    ScrollConsoleScreenBuffer(console, &scrollRect, NULL, scrollTarget, &fill);
    
    // Move the cursor to the top left corner too.
    csbi.dwCursorPosition.X = 0;
    csbi.dwCursorPosition.Y = 0;
    
    SetConsoleCursorPosition(console, csbi.dwCursorPosition);
}

//~ long: File Handling

// NOTE(long): It's ok for _scratch_ to collide with whatever arena in the caller
#define W32WidePath(name16, str, ...) Stmnt(ScratchBlock(_scratch_) \
                                            { \
                                                String16 name16 = Str16FromStr(_scratch_, (str)); \
                                                __VA_ARGS__; \
                                            })

function String OSReadFile(Arena* arena, String path)
{
    String result = {0};
    HANDLE file = 0;
    W32WidePath(path16, path, file = CreateFileW(path16.str,
                                                 GENERIC_READ, FILE_SHARE_READ, NULL,
                                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0));
    
    if (W32CheckHandle(file))
    {
        result = W32ReadFile(arena, file, (r1u64){0});
        CloseHandle(file);
    }
    
    return result;
}

function b32 OSWriteList(String path, StringList* data)
{
    b32 result = 0;
    HANDLE file = 0;
    W32WidePath(path16, path, file = CreateFileW(path16.str,
                                                 GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                                                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0));
    
    if (W32CheckHandle(file))
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
                if (!result)
                    goto END;
                
                ptr += actualWrite;
            }
        }
        
        END:
        CloseHandle(file);
    }
    
    return result;
}

function OS_Handle OS_FileOpen(String path, OS_AccessFlags flags, OS_Handle queue)
{
    OS_Handle result = {0};
    DWORD access = 0, share = 0, creation = OPEN_EXISTING, attributes = FILE_ATTRIBUTE_NORMAL;
    b32 is_async = OS_HandleIsValid(queue);
    
    if (flags & AccessFlag_Read)    { access |= GENERIC_READ; share |= FILE_SHARE_READ; }
    if (flags & AccessFlag_Write)   { access |= GENERIC_WRITE;share |= FILE_SHARE_WRITE|FILE_SHARE_DELETE;creation = CREATE_ALWAYS; }
    if (flags & AccessFlag_Append)  { access |= FILE_APPEND_DATA; creation = OPEN_ALWAYS; }
    if (flags & AccessFlag_Execute) { access |= GENERIC_EXECUTE; }
    if (is_async)                   { attributes |= FILE_FLAG_OVERLAPPED; }
    
    HANDLE file = 0;
    W32WidePath(path16, path, file = CreateFileW(path16.str, access, share, NULL, creation, attributes, 0));
    if (W32CheckHandle(file))
        if (!is_async || CreateIoCompletionPort(file, W32FromHandle(queue), (ULONG_PTR)file, 0))
            result = W32ToHandle(file);
    
    return result;
}

function void OS_FileClose(OS_Handle file)
{
    if (OS_HandleIsValid(file))
        if (!CloseHandle(W32FromHandle(file)))
            Errf("Failed to close file: %lu\n", GetLastError());
}

function u64 OS_FileRead(OS_Handle file, r1u64 rng, void* buffer)
{
    u64 result = 0;
    HANDLE handle = W32FromHandle(file);
    
    if (W32CheckHandle(handle))
    {
        r1u64 clampedRng;
        {
            u64 maxSize = 0;
            GetFileSizeEx(handle, (PLARGE_INTEGER)&maxSize); // TODO(long): Endianness
            
            clampedRng = R1U64(ClampTop(rng.min, maxSize), ClampTop(rng.max, maxSize));
            if (rng.min == 0 && rng.max == 0)
                clampedRng.max = maxSize;
        }
        
        u64 size = DimR1U64(clampedRng);
        if (size)
        {
            u64 bytesRead = 0;
            for (DWORD actualRead = 0, success = 1; bytesRead < size && success; bytesRead += actualRead)
            {
                u64 off = bytesRead + clampedRng.min;
                DWORD readAmount = (u32)ClampTop(size - bytesRead, MAX_U32);
                
                // NOTE(long): ReadFile will always zero out actualRead
                success = ReadFile(handle, PtrAdd(buffer, bytesRead), readAmount, &actualRead, &(OVERLAPPED){
                                       .Offset     = (off&0x00000000FFFFFFFFULL),
                                       .OffsetHigh = (off&0xffffffff00000000ULL) >> 32,
                                   });
            }
            result = bytesRead;
        }
    }
    
    return result;
}

function u64 OS_FileWrite(OS_Handle file, u64 pos, String data)
{
    u64 bytesWritten = 0, size = data.size;
    HANDLE handle = W32FromHandle(file);
    
    if (W32CheckHandle(handle) && size)
    {
        for (DWORD success = 1, actualWrite = 0; bytesWritten < size && success; bytesWritten += actualWrite)
        {
            u64 off = bytesWritten + pos;
            DWORD writeAmount = (u32)ClampTop(size - bytesWritten, MAX_U32);
            
            // NOTE(long): WriteFile will always zero out actualWrite
            success = WriteFile(handle, data.str + bytesWritten, writeAmount, &actualWrite, &(OVERLAPPED){
                                    .Offset     = (off&0x00000000ffffffffull),
                                    .OffsetHigh = (off&0xffffffff00000000ull) >> 32,
                                });
        }
    }
    
    return bytesWritten;
}

function OS_Handle OS_PushFileQueue(void)
{
    HANDLE result = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    return W32ToHandle(result);
}

typedef union W32FileEntry W32FileEntry;
union W32FileEntry
{
    W32FileEntry* next;
    struct
    {
        OVERLAPPED overlapped;
        void* data;
    };
};
global W32FileEntry* volatile freeEntry = 0;

function u64 OS_FileWaitAsync(OS_Handle queue, void** outUser, u64 timeoutMs)
{
    u32 result = 0;
    W32FileEntry* entry = 0;
    HANDLE iocpQueue = W32FromHandle(queue);
    if (!GetQueuedCompletionStatus(iocpQueue, &result, &(ULONG_PTR){0}, (LPOVERLAPPED*)&entry, W32TimeMS(timeoutMs)))
        Errf("Failed to wait for file: %lu\n", GetLastError());
    
    if (outUser)
        *outUser = entry->data;
    SLLAtomicPush(freeEntry, entry);
    return (u64)result;
}

function b32 OS_FileReadAsync(OS_Handle file, r1u64 rng, void* buffer, void* user)
{
    b32 result = 0;
    HANDLE handle = W32FromHandle(file);
    
    if (W32CheckHandle(handle))
    {
        r1u64 clampedRng;
        {
            u64 maxSize = 0;
            GetFileSizeEx(handle, (PLARGE_INTEGER)&maxSize); // TODO(long): Endianness
            
            clampedRng = R1U64(ClampTop(rng.min, maxSize), ClampTop(rng.max, maxSize));
            if (rng.min == 0 && rng.max == 0)
                clampedRng.max = maxSize;
        }
        
        u64 size = DimR1U64(clampedRng);
        if (size)
        {
            result = 1;
            u64 bytesRead = 0;
            
            for (DWORD actualRead = 0; bytesRead < size && result; bytesRead += actualRead)
            {
                W32FileEntry* entry = SLLAtomicPop(freeEntry);
                if (!entry) // @THREAD_SAFE(long): win32PermArena isn't thread-safe
                    entry = PushStruct(win32PermArena, W32FileEntry);
                
                u64 off = bytesRead + clampedRng.min;
                entry->overlapped.Offset     = (off&0x00000000FFFFFFFFULL);
                entry->overlapped.OffsetHigh = (off&0xffffffff00000000ULL) >> 32;
                entry->data = user;
                
                // NOTE(long): ReadFile will always zero out actualRead
                DWORD readAmount = (u32)ClampTop(size - bytesRead, MAX_U32);
                if (ReadFile(handle, PtrAdd(buffer, bytesRead), readAmount, &actualRead, &entry->overlapped))
                    result = bytesRead == size; // The file is already cached in memory
                else if (ALWAYS(GetLastError() == ERROR_IO_PENDING))
                    bytesRead += readAmount; // TODO(long): Test this on 4GB+ files
                else
                    result = 0;
            }
        }
    }
    
    return result;
}

internal FilePropertyFlags W32FilePropertyFlagsFromAttributes(DWORD attributes)
{
    FilePropertyFlags result = 0;
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        result |= FilePropertyFlag_IsFolder;
    if (attributes & FILE_ATTRIBUTE_HIDDEN)
        result |= FilePropertyFlag_IsHidden;
    return result;
}

internal DataAccessFlags W32AccessFromAttributes(DWORD attributes)
{
    DataAccessFlags result = 0;
    if (attributes & FILE_ATTRIBUTE_READONLY)
        result = DataAccessFlag_Read|DataAccessFlag_Execute;
    else
        result = DataAccessFlag_Read|DataAccessFlag_Write|DataAccessFlag_Execute;
    return result;
}

function FileProperties OSFileProperties(String path)
{
    FileProperties result = {0};
    WIN32_FILE_ATTRIBUTE_DATA attributes = {0};
    b32 success = 0;
    
    W32WidePath(path16, path, success = GetFileAttributesExW(path16.str, GetFileExInfoStandard, &attributes));
    if (success)
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

function FileProperties OS_FileProp(OS_Handle file)
{
    FileProperties result = {0};
    
    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(W32FromHandle(file), &info))
    {
        result = (FileProperties)
        {
            .size = ((u64)info.nFileSizeHigh << 32)|(u64)info.nFileSizeLow,
            .flags = W32FilePropertyFlagsFromAttributes(info.dwFileAttributes),
            
            .createTime = W32DenseTimeFromFileTime(&info.ftCreationTime),
            .modifyTime = W32DenseTimeFromFileTime(&info.ftLastWriteTime),
            .access = W32AccessFromAttributes(info.dwFileAttributes),
        };
    }
    
    return result;
}

function String  OSGetExeDir(void) { return win32BinaryPath; }
function String OSGetUserDir(void) { return win32UserPath; }
function String OSGetTempDir(void) { return win32TempPath; }
function String OSGetCurrDir(Arena* arena)
{
    String result = {0};
    ScratchBlock(scratch, arena)
    {
        DWORD size = GetCurrentDirectoryW(0, 0);
        u16* buffer = PushArrayNZ(arena, u16, size);
        size = GetCurrentDirectoryW(size, buffer);
        result = StrFromStr16(arena, Str16(buffer, size));
    }
    return result;
}

function b32 OSDeleteFile(String path)
{
    b32 result = 0;
    W32WidePath(path16, path, result = DeleteFileW(path16.str));
    return result;
}

function b32 OSRenameFile(String oldName, String newName)
{
    b32 result = 0;
    ScratchBlock(scratch)
    {
        // @W32WidePath
        String16 o16 = Str16FromStr(scratch, oldName);
        String16 n16 = Str16FromStr(scratch, newName);
        
        // NOTE(long): Can't move a directory across drives
        result = MoveFileW(o16.str, n16.str);
    }
    return result;
}

function b32 OSCreateDir(String path)
{
    b32 result = 0;
    ScratchBlock(scratch)
    {
        WIN32_FILE_ATTRIBUTE_DATA attributes = {0};
        String16 wpath = Str16FromStr(scratch, path); // @W32WidePath
        GetFileAttributesExW(wpath.str, GetFileExInfoStandard, &attributes);
        
        result = attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        if (!result)
            result = CreateDirectoryW(wpath.str, 0);
    }
    return result;
}

function b32 OSDeleteDir(String path)
{
    b32 result = 0;
    W32WidePath(wpath, path, result = RemoveDirectoryW(wpath.str));
    return result;
}

//~ long: File Iteration

typedef struct W32FileIter W32FileIter;
struct W32FileIter
{
    W32FileIter* next;
    String path;
    HANDLE handle;
    u8* buffer;
    FILE_ID_EXTD_DIR_INFO* info;
};
StaticAssert(ArrayCount(MemberOf(OSFileIter, v)) >= sizeof(W32FileIter), w32fileiter);
#define W32GetIter(iter) ((W32FileIter*)(iter)->v)

#define FILE_ITER_BUFFER_SIZE KiB(64)
internal b32 W32FileIterInit(Arena* arena, W32FileIter* iter, String path)
{
    b32 result = 0;
    W32WidePath(wpath, path, iter->handle = CreateFileW(wpath.str, FILE_LIST_DIRECTORY,
                                                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                                        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0));
    iter->path = path;
    
    if (W32CheckHandle(iter->handle))
    {
        TempArena temp = TempBegin(arena);
        iter->buffer = PushArray(arena, u8, FILE_ITER_BUFFER_SIZE);
        if (GetFileInformationByHandleEx(iter->handle, FileIdExtdDirectoryRestartInfo,
                                         iter->buffer, FILE_ITER_BUFFER_SIZE))
        {
            result = 1;
            iter->info = (FILE_ID_EXTD_DIR_INFO*)iter->buffer;
        }
        else TempEnd(temp);
    }
    
    return result;
}

internal void W32FileIterEnd(W32FileIter* iter)
{
    if (W32CheckHandle(iter->handle))
        CloseHandle(iter->handle);
}

function OSFileIter FileIterInit(Arena* arena, String path, OSFileIterFlags flags)
{
    OSFileIter result = { .flags = flags };
    W32FileIterInit(arena, W32GetIter(&result), path);
    return result;
}

function void FileIterEnd(OSFileIter* iter)
{
    W32FileIterEnd(W32GetIter(iter));
}

function b32 FileIterNext(Arena* arena, OSFileIter* iter)
{
    b32 result = 0;
    W32FileIter* w32Iter = W32GetIter(iter);
    
    if (W32CheckHandle(w32Iter->handle))
    {
        while (!(iter->flags & FileIterFlag_Done))
        {
            FILE_ID_EXTD_DIR_INFO* info = w32Iter->info;
            ULONG attributes = info->FileAttributes;
            String16 fileName = Str16(info->FileName, info->FileNameLength/2);
            
            // Check for . and ..
            b32 emit = fileName.str[0] != '.' || (fileName.size > 1 && !(fileName.str[1] == '.' && fileName.size == 2));
            b32 recursive = 0;
            
            if (emit)
            {
                if ((attributes & FILE_ATTRIBUTE_HIDDEN) && (iter->flags & FileIterFlag_SkipHidden))
                    emit = 0;
                
                else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    recursive = iter->flags & FileIterFlag_Recursive;
                    emit = !(iter->flags & FileIterFlag_SkipFolders);
                }
                
                else if (iter->flags & FileIterFlag_SkipFiles)
                    emit = 0;
            }
            
            // NOTE(long): Must do this before FindNextFile because it will overwrite data and fileName
            if (emit || recursive)
            {
                iter->name = StrFromStr16(arena, fileName);
                iter->path = w32Iter->path;
            }
            
            if (emit)
            {
                iter->props = (FileProperties)
                {
                    .size = ((u64)info->EndOfFile.HighPart << 32) | info->EndOfFile.LowPart,
                    .flags = W32FilePropertyFlagsFromAttributes(attributes),
                    .access = W32AccessFromAttributes(attributes),
                    
                    // TODO(long): Does type-punning work?
                    .createTime = W32DenseTimeFromFileTime((FILETIME*)&info->CreationTime),
                    .modifyTime = W32DenseTimeFromFileTime((FILETIME*)&info->LastWriteTime),
                };
            }
            
            // Recursively iterate all the files and subfolders
            if (recursive)
            {
                TempArena temp = TempBegin(arena);
                W32FileIter* next = PushStruct(arena, W32FileIter);
                *next = *w32Iter;
                w32Iter->next = next;
                w32Iter->buffer = 0;
                
                String subdir = StrPushf(arena, "%.*s\\%.*s", StrExpand(iter->path), StrExpand(iter->name));
                if (!W32FileIterInit(arena, w32Iter, subdir))
                {
                    *w32Iter = *next;
                    TempEnd(temp);
                    goto REP;
                }
            }
            
            // Increment the iter
            else
            {
                REP:
                if (w32Iter->info->NextEntryOffset != 0) // Go to next file
                    w32Iter->info = (FILE_ID_EXTD_DIR_INFO*)((u8*)w32Iter->info+w32Iter->info->NextEntryOffset);
                
                else
                {
                    w32Iter->info = (FILE_ID_EXTD_DIR_INFO*)w32Iter->buffer;
                    // Check whether there are more files to fetch.
                    if (!GetFileInformationByHandleEx(w32Iter->handle, FileIdExtdDirectoryInfo,
                                                      w32Iter->buffer, FILE_ITER_BUFFER_SIZE))
                    {
                        Assert(GetLastError() == ERROR_NO_MORE_FILES);
                        
                        if (w32Iter->next)
                        {
                            // Pop and repeat
                            W32FileIterEnd(w32Iter);
                            *w32Iter = *w32Iter->next;
                            goto REP;
                        }
                        else iter->flags |= FileIterFlag_Done;
                    }
                }
            }
            
            result |= emit;
            if (emit)
                break;
        }
    }
    
    return result;
}
#undef FILE_ITER_BUFFER_SIZE

//~ long: Libraries

function OS_Handle OSLoadLib(String path)
{
    OS_Handle result = {0};
    W32WidePath(wpath, path, result.v[0] = (u64)LoadLibraryW(wpath.str));
    return result;
}

function VoidFunc* OSGetProc(OS_Handle lib, char* name)
{
    HMODULE module = (HMODULE)lib.v[0];
    VoidFunc* result = (VoidFunc*)(GetProcAddress(module, name));
    return result;
}

function void OSFreeLib(OS_Handle lib)
{
    HMODULE module = (HMODULE)lib.v[0];
    FreeLibrary(module);
}

//~ long: Entropy

function void OSGetEntropy(void* data, u64 size)
{
    HCRYPTPROV prov = 0;
    CryptAcquireContextW(&prov, 0, 0, PROV_DSS, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(prov, (u32)ClampTop(size, MAX_U32), (BYTE*)data);
    CryptReleaseContext(prov, 0);
}

//~ long: Clipboard

function void OSSetClipboard(String string)
{
    if (OpenClipboard(0))
    {
        EmptyClipboard();
        HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, string.size + 1);
        u8* buffer = handle ? (u8*)GlobalLock(handle) : 0;
        if (buffer)
        {
            CopyMem(buffer, string.str, string.size);
            buffer[string.size] = 0;
            GlobalUnlock(handle);
            SetClipboardData(CF_TEXT, handle);
        }
        CloseClipboard();
    }
}

function String OSGetClipboard(Arena *arena)
{
    String result = {0};
    if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(0))
    {
        HANDLE handle = GetClipboardData(CF_TEXT);
        u8* buffer = handle ? (u8*)GlobalLock(handle) : 0;
        if (buffer)
        {
            result = StrCloneCStr(arena, buffer);
            GlobalUnlock(handle);
        }
        CloseClipboard();
    }
    return result;
}

//~ long: System Info

function SysInfo OSGetSysInfo(void)
{
    return win32SysInfo;
}

//~ NOTE(long): Processes/Threads

// https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
function OS_Handle OS_ProcessLaunch(String cmd, OS_ProcessParams* params)
{
    OS_Handle result = {0};
    ScratchBegin(scratch);
    
    // UTF-8 -> UTF-16
    // NOTE(long): If the string is empty, all of these must be null
    WCHAR* cmd16 = 0,* dir16 = 0,* env16 = 0;
    {
        cmd16 = Str16FromStr(scratch, cmd).str;
        dir16 = Str16FromStr(scratch, params->path).str;
        if (params->env.totalSize)
        {
            String env = StrJoin(scratch, &params->env, .mid = StrLit("\0"), .post = StrLit("\0"));
            env16 = Str16FromStr(scratch, env).str;
        }
    }
    
    // STD Handles
    BOOL inheritHandles = 0;
    STARTUPINFOW startupInfo = {sizeof(startupInfo)};
    {
        if (OS_HandleIsValid(params->stdOutHandle))
        {
            startupInfo.hStdOutput = W32FromHandle(params->stdOutHandle);
            startupInfo.dwFlags |= STARTF_USESTDHANDLES;
            inheritHandles = 1;
        }
        
        if (OS_HandleIsValid(params->stdErrHandle))
        {
            startupInfo.hStdError = W32FromHandle(params->stdErrHandle);
            startupInfo.dwFlags |= STARTF_USESTDHANDLES;
            inheritHandles = 1;
        }
        
        if (OS_HandleIsValid(params->stdInHandle))
        {
            startupInfo.hStdInput = W32FromHandle(params->stdInHandle);
            startupInfo.dwFlags |= STARTF_USESTDHANDLES;
            inheritHandles = 1;
        }
    }
    
    PROCESS_INFORMATION processInfo = {0};
    if (CreateProcessW(NULL, // no module name
                       cmd16,
                       NULL, NULL, inheritHandles, // inheritance
                       CREATE_UNICODE_ENVIRONMENT|(!!params->consoleless*CREATE_NO_WINDOW),
                       env16, dir16, &startupInfo, &processInfo))
    {
        result = W32ToHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
    else ErrorFmt("Failed to create process: %lu", GetLastError());
    
    ScratchEnd(scratch);
    return result;
}

function b32 OS_ProcessJoin(OS_Handle handle, u64 timeoutMs)
{
    HANDLE process = W32FromHandle(handle);
    DWORD waitResult = WaitForSingleObject(process, W32TimeMS(timeoutMs));
    b32 result = waitResult == WAIT_OBJECT_0;
    if (result)
        CloseHandle(process);
    return result;
}

function void OS_ProcessDetach(OS_Handle handle)
{
    CloseHandle(W32FromHandle(handle));
}

function b32 OS_ProcessExec(String cmd, OS_ProcessParams* params)
{
    b32 result = 0;
    ScratchBlock(scratch)
    {
        cmd = StrPushf(scratch, "cmd.exe /C %.*s", StrExpand(cmd));
        OS_Handle handle = OS_ProcessLaunch(cmd, params);
        result = OS_ProcessJoin(handle, INFINITE);
        OS_ProcessDetach(handle);
    }
    return result;
}

//~ NOTE(long): Threads

function OS_Handle OS_ThreadLaunch(OS_ThreadFunc* func, void* params)
{
    uptr stackSize = 0;
    u32 threadId = 0;
    HANDLE handle = CreateThread(0, stackSize, (LPTHREAD_START_ROUTINE)func, params, 0, &threadId);
    return W32ToHandle(handle);
}

function b32 OS_ThreadJoin(OS_Handle handle, u64 timeoutMs)
{
    HANDLE thread = W32FromHandle(handle);
    DWORD waitResult = WaitForSingleObject(thread, W32TimeMS(timeoutMs));
    b32 result = waitResult == WAIT_OBJECT_0;
    if (result)
        CloseHandle(thread);
    return result;
}

function void OS_ThreadDetach(OS_Handle handle)
{
    CloseHandle(W32FromHandle(handle));
}
#endif
