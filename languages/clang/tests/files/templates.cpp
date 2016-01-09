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

/// "type" : { "toString" : "Class_volatile_const< int >" }
Class_volatile_const<int> instance;

/// "type" : { "toString" : "VariadicTemplate< int, double, bool >", "EXPECT_FAIL": {"toString": "No way to get variadic template arguments with LibClang"} }
VariadicTemplate<int, double, bool> variadic;

/// "type" : { "toString" : "TemplateTest< const TemplateTest< int, 100 >, 30 >" }
TemplateTest<const TemplateTest<int, 100>, 30> tst;

template<class Type>
void test()
{
    /// "type" : { "toString" : "const volatile auto" }
    const volatile auto type = Type();
}
