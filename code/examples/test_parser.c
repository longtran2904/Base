int decl1 = 10;
int decl2 = 100;
float decl3 = 10000;
char decl4 = 50;

Foo*** var;
Foo foo(i32 a, f32 b, Foo c, Foo array[], Bar* ptr);
Bar* bar(Bar* blah, Foo foo);

int*[5];

int pointerInArray(/*int[*]*/);
int specifier1InArray(int [static 10]);
int specifier2InArray(int[const volatile]);
const int* volatile bar;

int e1, * e2, ** *** * e3;
// NOTE(long): f4 is nonstandard extension warning: qualifiers after comma in declarator list are ignored
static const unsigned int* qualifierPtr1 = 0, qualifierPtr2 = 50, *** restrict * qualifierPtr3 = 0, volatile qualifierPtr4 = 100;

int*** const * ptr1[1][2][3];
float** (**(*ptr2[1])[2][3])[4];
char (*(*ptr3[3])())[5];
char * const (*(* const ptr4)[5])(int);
int (*(*ptr5)(const void *))[3];
int* ptr6,** ptr7, ptr8[10], ** ptr9[20], ** (**(*ptr10[1])[2][3])[4];
void func1( int ), * (func2)( int ), (*funcPtr)(void);
int (*(*ptr11)(void))[3], (*ptr12(void))[3];
int func3(int[]), func4(char arg1(double), int arg2(int arg, float, Foo), Foo** arg3 = foo), func5(int(void)), func6(const int[10]);

struct AStruct
{
    int a;
    int b;
};

struct BStruct
{
    int array[10];
    char ** const * const ptr;
    int a1, a2;
    int* b1, b2, **** *** ** * b3;
};

typedef struct CStruct CStruct;
struct CStruct
{
    float** (**(*c1[1])[2][3])[4];
    char c2[1024];
};

typedef struct DStruct
{
    int a;
} DStruct;

struct EStruct
{
    int a;
};

typedef struct
{
    int a;
} FStruct;

static struct
{
    int a;
} myGStruct, * const * volatile *myGPtr;

struct HStruct
{
    int a;
}** const myHStruct = 0, myPtr;

typedef struct XStruct
{
    int x;
} YStruct;

struct ZStruct
{
    struct
    {
        i32 a, ** * b;
    };
    struct
    {
        float f;
        float f2[10];
    };
    struct HStruct h;
};

struct WStruct
{
    struct
    {
        i32 a, ** * b;
    };
    
    struct
    {
        float f;
        struct
        {
            char c[1024];
        };
        float f2[10];
    };
    
    b32 d;
} wVar1, **wVar2;

enum AEnum { FooA, BarA };
typedef enum BEnum { FooB } BEnum;
typedef enum CEnum CEnum; enum CEnum { FooC };

typedef enum
{
    FooD,
    BarD = 10,
} DEnum;

enum { FooE } EEnum;

enum FEnum
{
    F1 = foo / bar * blah,
    F2,// = {},
    F3,// = Func(&(Type){ .a = 100*100, .b = 0, }, baz),
    F4 = 100
};

int * ** *** **** Func1(int ** a, float b, char c[1024], char** d[10], char ** const * volatile e[1][2][3]);

struct {
    int**** a[1][2][3];
    float** (**(*b[1])[2][3])[4];
    char (*(*c[3])())[5];
    char * const (*(* const d)[5])(int);
    int (*(*e)(const void *))[3];
    int* f1,** f2, f3[10], ** f4[20], ** (**(*f5[1])[2][3])[4];
    int (* (**funcPtr)(void))[3][4];
    int (* (**funcArr[10])(void))[3][4];
}* Func2(struct
         {
             struct { i32 a, ** * b; };
             struct { float f; float f2[10]; };
             struct HStruct h;
         }* arg1, int arg2);// { return 0; }

struct ReturnType {
    int**** a[1][2][3];
    float** (**(*b[1])[2][3])[4];
    char (*(*c[3])())[5];
    char * const (*(* const d)[5])(int);
    int (*(*e)(const void *))[3];
    int* f1,** f2, f3[10], ** f4[20], ** (**(*f5[1])[2][3])[4];
    int * (**funcPtr)(void**[1], int[2]);
} Func3(// NOTE(long): warning: named type definition in parentheses
        // If you ignore this warning, everything works fine
        // You can use this type when declaring other arguments/locals
        struct Argument
        {
            struct { i32 a, ** * b; };
            struct { float f; float f2[10]; };
            struct HStruct h;
        }* arg1, int arg2
        
        // NOTE(long): Because arguments can be anonymous, they can't differentiate between
        // a function pointer decl and a paren-containing argument, even though the same
        // argument can be parsed in other cases (field, variable, etc)
        
        //, int * (**funcPtr)(void**[1], int[2])
        );

void Func4(struct { int a; } a, struct { float b; }** volatile *, int*(float), const int, volatile Foo, struct A, Bar* const* volatile*);

a / b * c;
a << b * c - d + e - f << g;
foo = a * b - c + d;

foo = a * -b + *c / d[0];
a << b - c * -d - e;
a + *&b++-- = 5;

1 + (b)(c)(d - 10) * (e)(f)&g++;
a = b - c * d(1)(2, 3) + (e << f) * (int)g;

a + b ? c - d : e * 5;
1 ? 2 : 3 ? 4 : 5 + 6 ? 7 : 8 = 9;
(((a ? b : c) ? d : e + f) ? g : h) = i;
a + (b + (c + (d + e))++);
