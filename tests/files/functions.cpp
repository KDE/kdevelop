class foo
{
    /// "isStatic" : false,
    /// "isVirtual" : true
    virtual void bla() {}
    /// "isStatic" : false,
    /// "isVirtual" : true
    virtual void bar();
    /// "isStatic" : true,
    /// "isVirtual" : false
    static void bla2() {}
    /// "isStatic" : true,
    /// "isVirtual" : false
    static void bar2();
};

/// "isVirtual" : false
void bar2() {}
/// "isVirtual" : false
void bar1();
