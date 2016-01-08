// this file is allowed to have erroneous code, which is still parseable

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
