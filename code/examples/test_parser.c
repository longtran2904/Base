int a = 10;
int b = 100;
float c = 10000;
char d = 50;

int e1, * e2, ** *** * e3;
int* f1 = 0, f2, *** * f3 = 1, f4;

Foo*** var;

Foo foo(i32 a, f32 b, Foo c, Foo array[], Bar* ptr);
Bar* bar(Bar* blah, Foo foo);

int*[5];

int (*funcPtr)();

int pointerInArray(int[*]);
int specifier1InArray(int [static 10]);
int specifier2InArray(int[const volatile]);
_Atomic unsigned long long int const volatile *restrict const foo[10][20][50];
const int* volatile bar;

int*** const * a[1][2][3];
float** (**(*b[1])[2][3])[4];
char (*(*c[3])())[5];
char * const (*(* const d)[5])(int);
int (*(*e)(const void *))[3];
int* f1,** f2, f3[10], ** f4[20], ** (**(*f5[1])[2][3])[4];
void func1( int ), * (func2)( int ), (*funcPtr)(void);
int (*(*foo1)(void))[3], (*foo2(void))[3];
int bar1(int[]), bar2(char arg1(double), int arg2(int arg, float), Foo** arg3 = foo), bar3(int(void)), bar4(const int[10]);

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
    char c[1024];
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

struct
{
    int a;
} myGStruct, ***myGPtr;

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

union AUnion
{
    int a;
    int b;
};

union BUnion
{
    int array[10];
    char ** const * const ptr;
    int a1, a2;
    int* b1, b2, **** *** ** * b3;
};

typedef union CUnion CUnion;
union CUnion
{
    char c[1024];
};

typedef union DUnion
{
    int a;
} DUnion;

union EUnion
{
    int a;
};

typedef union
{
    int a;
} FUnion;

union
{
    int a;
} GUnion;

union HUnion
{
    int a;
} myHUnion;

typedef union XUnion
{
    int x;
} YUnion;

union ZUnion
{
    union
    {
        i32 a, ** * b;
    };
    union
    {
        float f;
        float f2[10];
    };
    union HUnion h;
};

union WUnion
{
    union
    {
        i32 a, ** * b;
    };
    
    union
    {
        float f;
        union
        {
            char c[1024];
        };
        float f2[10];
    };
    
    b32 d;
};

//enum AEnum { FooA, BarA };
//typedef enum BEnum { FooB } BEnum;
//typedef enum CEnum CEnum; enum CEnum { FooC };

//typedef enum
//{
//FooD,
//BarD = 10,
//} DEnum;

//enum { FooE } EEnum;

//enum FEnum
//{
//F1 = foo * bar / blah,
//F2 = {},
//F3 = Func(&(Type){ .a = 100*100, .b = 0, }, baz),
//F4 = 100
//};

//void Func1();
//void* Func2();
//void* Func3(void);
//void* Func4(void*);
//void Func5(void* a);
//int * ** *** **** Func6(int ** a, float b, char c[1024], char** d[10], char ** const * volatile e[1][2][3]);

//struct {
//int**** a[1][2][3];
//float** (**(*b[1])[2][3])[4];
//char (*(*c[3])())[5];
//char * const (*(* const d)[5])(int);
//int (*(*e)(const void *))[3];
//int* f1,** f2, f3[10], ** f4[20], ** (**(*f5[1])[2][3])[4];
//int (* (**funcPtr)(void))[3][4];
//int (* (**funcArr[10])(void))[3][4];
//}* Func7(struct
//{
//struct { i32 a, ** * b; };
//struct { float f; float f2[10]; };
//struct HStruct h;
//}* a, int b) { return 0; }

//struct ReturnType {
//int**** a[1][2][3];
//float** (**(*b[1])[2][3])[4];
//char (*(*c[3])())[5];
//char * const (*(* const d)[5])(int);
//int (*(*e)(const void *))[3];
//int* f1,** f2, f3[10], ** f4[20], ** (**(*f5[1])[2][3])[4];
//int * (**funcPtr)(void**[1], int[2]);
//} Func8(struct Argument
//{
//struct { i32 a, ** * b; };
//struct { float f; float f2[10]; };
//struct HStruct h;
//}* a, int b, int * (**funcPtr)(void**[1], int[2]));

//int**** a[1][2][3];
//float** (**(*b[1])[2][3])[4];
//char (*(*c[3])())[5];
//char * const (*(* const d)[5])(int) = 0;
//int (*(*e)(const void *))[3];
//int* f1,** f2, f3[10], ** f4[20], ** (**(*f5[1])[2][3])[4];
//void func1( int ), * (func2)( int ), (*funcPtr)(void);

/*function MG_ExprResult CL_ParseExpr(Arena* arena, MD_Node* first, MD_Node* opl)
{
MG_ExprResult parse = {&mg_exprNil};
MG_Expr* currExpr = &mg_exprNil;

for (MD_EachNode(it, first))
{
if (CL_ExprIsAtom(node))
{
MG_Expr* atom = MG_PushExpr(arena, MG_ExprOpKind_Null, node);
if (currExpr == &mg_exprNil)
{
Assert(it == first);
currExpr = MG_PushExpr(arena, MG_ExprOpKind_Null, node);
}

Assert(currExpr->right == mg_exprNil);
currExpr->right = atom;
atom->parent = currExpr;
}

else if (!md_node_is_nil(it->first))
{
Assert();
MG_ExprResult subparse = MG_ParseExpr(arena, it->first, &md_nil_node, 0);
atom = subparse.root;
md_msg_list_concat_in_place(&parse.msgs, &subparse.msgs);
it = it->next;
}

else
{
MG_ExprOpKind op = CL_ExprKind(it);
Assert(op);
if (op)
{
MG_Expr* expr = MG_PushExpr(arena, op, it);
MG_Expr* parent = currExpr;
i32 prec = CL_ExprPrec(op);

for (MG_Expr* tail = parent; tail != &mg_exprNil; parent = tail, tail = tail->parent)
{
i32 parentPrec = CL_ExprPrec(parent->op);
if (parentPrec < prec && !(parentPrec == prec && CL_ExprIsLR(parent->op)))
break;
if (!CL_ExprIsClosed(parent))
break;
}

if (parent != &mg_exprNil)
{
Assert(parent->right != &mg_exprNil || CL_ExprIsPrefix(op));
Assert(parent->left  != &mg_exprNil || CL_ExprIsUnary(parent->op));

expr->parent = parent;
expr->left = parent->right;
expr->left->parent = expr;
}
else Assert(it == first);

currExpr = expr;
}
}
}

while (currExpr->parent != mg_exprNil)
currExpr = currExpr->parent;
parse.root = currExpr;
return parse;
}*/
