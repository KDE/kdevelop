struct Base
{
    /// "type" : { "toString" : "Base::Class" },
    /// "useCount" : 1
    class Class;
    /// "identifiedTypeDeclaration" : { "isDefinition" : true }
    Class *d1;

    /// "type" : { "toString" : "Base::Struct" },
    /// "useCount" : 1
    struct Struct;
    /// "identifiedTypeDeclaration" : { "isDefinition" : true }
    Struct *d2;
};

class Base::Class {};

struct Base::Struct {};

/// "type" : { "toString" : "Foo" },
/// "useCount" : 1
class Foo;
/// "identifiedTypeDeclaration" : { "isDefinition" : true }
Foo* f;

class Foo {};
