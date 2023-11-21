/* date = July 25th 2022 1:11 am */

#ifndef ENUM_NAME
#error ENUM_NAME is undefined
#endif

#ifndef ENUM_VALUE
#error ENUM_VALUE is undefined
#endif

#define X(n) Concat(ENUM_NAME, Concat(_, n)),
typedef enum ENUM_NAME
{
    X(None)
        ENUM_VALUE(X)
        X(Count)
} ENUM_NAME;

#undef X
#define X(n) ConstStr(Stringify(n)),
global String Concat(ENUM_NAME, _names)[] =
{
    X(None)
        ENUM_VALUE(X)
};
#undef X

#undef ENUM_NAME
#undef ENUM_VALUE
