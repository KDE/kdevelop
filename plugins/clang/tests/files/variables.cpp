class foo
{
    /// "isStatic" : true
    static int m;
    /// "isStatic" : false
    int b;
};

/// "internalContext" : { "localDeclarationCount" : 1, "findDeclarations" : { "" : {"range" : "[(9, 10), (9, 10)]" } } }
void f(int);
/// "internalContext" : { "localDeclarationCount" : 1}
void f2(int a);
