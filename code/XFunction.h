/* date = July 31st 2022 11:44 am */

#ifndef FUNCTION_VALUE
#error FUNCTION_VALUE is undefined
#endif

#ifndef FUNCTION_PREFIX
#define FUNCTION_PREFIX
#endif

#ifndef POINTER_PREFIX
#define POINTER_PREFIX
#endif

#define X(r, n, p) typedef r Concat(FUNCTION_PREFIX, Concat(n, Func)) p;
FUNCTION_VALUE(X)
#undef X

#define X(r, n, p) global Concat(FUNCTION_PREFIX, Concat(n, Func))* Concat(POINTER_PREFIX, n) = 0;
FUNCTION_VALUE(X)
#undef X

#undef FUNCTION_VALUE

#ifndef KEEP_PREFIX
#undef FUNCTION_PREFIX
#undef POINTER_PREFIX
#endif
