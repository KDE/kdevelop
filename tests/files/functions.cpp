class foo
{
    /// "isStatic" : false,
    /// "isVirtual" : true,
    /// "isAbstract" : false
    virtual void bla() {}
    /// "isStatic" : false,
    /// "isVirtual" : true,
    /// "isAbstract" : false
    virtual void bar();
    /// "isStatic" : true,
    /// "isVirtual" : false,
    /// "isAbstract" : false
    static void bla2() {}
    /// "isStatic" : true,
    /// "isVirtual" : false,
    /// "isAbstract" : false
    static void bar2();

    /// "isStatic" : false,
    /// "isVirtual" : true,
    /// "isAbstract" : true
    virtual void bar3() = 0;

    /// "type" : { "toString"  : "function void () const", "isConst" : true }
    void constMethod() const;
    /// "type" : { "toString"  : "function void ()", "isConst" : false }
    void nonConstMethod();
};

/// "isVirtual" : false
void bar2() {}
/// "isVirtual" : false
void bar1();

/// "definition" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void someFunc(int a);
/// "declaration" : { "null" : true }
void someFunc(int a, int f) { }
/// "declaration" : { "internalContext" : { "type" : "Function", "localDeclarationCount" : 1 } }
void someFunc(int a) { }
