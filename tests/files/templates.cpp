/// "toString" : "struct myTemplate",
/// "kind" : "Type"
template<class T>
struct myTemplate {};

/// "toString" : "struct myTemplateChild",
/// "kind" : "Type"
struct myTemplateChild : myTemplate<int> { };

// Test for CXType_DependentSizedArray
template<typename T>
struct Class
{
    /// "toString" : "char[] data"
    char data[10 * sizeof(T)];
};

template<typename T>
struct Class_volatile_const
{};

template <typename T, int i = 100>
class TemplateTest
{};

template<typename T, typename... Targs>
class VariadicTemplate {};

/// "type" : { "toString" : "TypeAliasTemplate", "EXPECT_FAIL": {"toString": "TypeAliasTemplateDecl is not accessible through LibClang"} }
template <typename T>
using TypeAliasTemplate = T;

/// "type" : { "toString" : "Class_volatile_const< int >" }
Class_volatile_const<int> instance;
/// "type" : { "toString" : "TemplateTest< const TemplateTest< int, 100 >&, 30 >" }
TemplateTest<const TemplateTest<int, 100>&, 30> tst;
/// "type" : { "toString" : "VariadicTemplate< int, double, bool >", "EXPECT_FAIL": {"toString": "No way to get variadic template arguments with LibClang"} }
VariadicTemplate<int, double, bool> variadic;

/*This example used to crash while building the type of Bar*/
/// "type" : { "toString" : "Bar" }
template <typename T>
class Bar
{
    /// "type" : { "toString" : "function void (int)" }
    void foo(UnknownType);
    /// "returnType" : { "toString" : "int" }
    UnknownType foo();
};
