/// "toString" : "struct myTemplate",
/// "kind" : "Type"
template<class T>
struct myTemplate {};

/// "toString" : "struct myTemplateChild",
/// "kind" : "Type"
struct myTemplateChild : myTemplate<int> { };

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
