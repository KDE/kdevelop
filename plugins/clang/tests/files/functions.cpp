class Base
{
public:
    /// "isNoexcept" : false,
    /// "isStatic" : false,
    /// "isVirtual" : true,
    /// "isAbstract" : false
    virtual void bla() {}
    /// "isNoexcept" : false,
    /// "isStatic" : false,
    /// "isVirtual" : true,
    /// "isAbstract" : false
    virtual void bar();
    /// "isNoexcept" : false,
    /// "isStatic" : true,
    /// "isVirtual" : false,
    /// "isAbstract" : false
    static void bla2() {}
    /// "isNoexcept" : false,
    /// "isStatic" : true,
    /// "isVirtual" : false,
    /// "isAbstract" : false
    static void bar2();

    /// "isNoexcept" : false,
    /// "isStatic" : false,
    /// "isVirtual" : true,
    /// "isAbstract" : true,
    /// "isFinal" : false
    virtual void bar3() = 0;

    /// "isNoexcept" : true,
    /// "isStatic" : false,
    /// "isVirtual" : false,
    /// "isAbstract" : false,
    /// "isFinal" : false
    void bar4() noexcept;

    /// "type" : { "toString"  : "function void () const", "isConst" : true }
    void constMethod() const;
    /// "type" : { "toString"  : "function void ()", "isConst" : false }
    void nonConstMethod();
};

class Derived : public Base
{
public:
    /// "isNoexcept" : false,
    /// "isStatic" : false,
    /// "isVirtual" : true,
    /// "isAbstract" : false,
    /// "isFinal" : true
    void bar3() final;
};

/// "isNoexcept" : false
void bar1() {}
/// "isNoexcept" : true
void bar2() noexcept {}

/// "definition" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void someFunc(int a);
/// "declaration" : { "null" : true }
void someFunc(int a, int f) { }
/// "declaration" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void someFunc(int a) { }

// Pointer-to-member functions
/// "type" : { "toString"  : "function void ()*" }
void (Base::*f1)();
/// "type" : { "toString"  : "function void (int, int)*" }
void (Base::*f2)(int a, int b);

#define STDCALL __stdcall
/// "type" : { "toString" : "function void (int)" }
void STDCALL bar3(int a);
/// "definition" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void STDCALL bar3(int a);
/// "type" : { "toString" : "function void ()" }
void __pascal bar4();

/// "type" : { "toString" : "function void (...)" }
void variadic1(...);
/// "type" : { "toString" : "function int (int, ...)" }
int variadic2(int, ...);
/// "type" : { "toString" : "function void (int, ...)" }
void variadic3(int a...);
