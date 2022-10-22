/// "toString" : "struct myTemplate< T >",
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

/// "toString" : "struct Class_volatile_const< T >"
template<typename T>
struct Class_volatile_const
{};

/// "toString" : "class TemplateTest< T, i >"
template <typename T, int i = 100>
class TemplateTest
{};

/// "toString" : "class Partial< Foo >"
template<typename Foo>
class Partial : TemplateTest<Foo, 42> {};

/// "toString" : "class VariadicTemplate< T, Targs >"
template<typename T, typename... Targs>
class VariadicTemplate {};

/// "type" : { "toString" : "Class_volatile_const< int >" }
Class_volatile_const<int> instance;

/// "type" : { "toString" : "TemplateTest< const TemplateTest< int, 100 >, 30 >" }
TemplateTest<const TemplateTest<int, 100>, 30> tst;

/// "toString" : "class BoolTParam< X >"
template<bool X>
class BoolTParam{};

/// "type" : { "toString" : "BoolTParam< 1 < 2 >" }
BoolTParam<1<2> ltOperatorTArg;

/// "type" : { "toString" : "BoolTParam< 1 <= -1 >" }
BoolTParam<1<=-1> lteOperatorTArg;

/// "type" : { "toString" : "TemplateTest< TemplateTest< BoolTParam< 1 < 0 >, 1 << 5 > >" }
TemplateTest < TemplateTest < BoolTParam < 1 < 0 > , 1 << 5 > > spacesEverywhere;

/// "toString" : "class SpacedDefaultParam< int >"
template < int =5<<2 > class SpacedDefaultParam {};

/// "type" : { "toString" : "SpacedDefaultParam< 20 >" }
SpacedDefaultParam spacedDefaultParam;

/// "type" : { "toString" : "SpacedDefaultParam<  >" }
SpacedDefaultParam<> spacedDefaultParamEmpty;

/// "type" : { "toString" : "SpacedDefaultParam< 8 << 5 >" }
SpacedDefaultParam<8<<5> spacedDefaultParamLeftShift;

/// "type" : { "toString" : "SpacedDefaultParam< (1 > 9) >" }
SpacedDefaultParam< (1>9) > spacedDefaultParamGt;

/// "toString" : "void test< Type >()",
/// "EXPECT_FAIL": {"toString": "No way to get template parameters with libclang, and display name would duplicate the signature"}
template<class Type>
void test()
{
    /// "type" : { "toString" : "const volatile auto" }
    const volatile auto type = Type();
}

namespace Foo {
/// "toString" : "struct Bar< T >"
template<typename T>
struct Bar {
    /// "toString" : "class Nested< T2 >"
    template<typename T2>
    class Nested {};
};
}

/// "type" : {
///     "toString" : "Foo::Bar< int >::Nested< float >",
///     "EXPECT_FAIL": {"toString": "The parent specialization information is lost, i.e. Bar<int> becomes Bar<T>"}
/// }
Foo::Bar<int>::Nested<float> asdf;
