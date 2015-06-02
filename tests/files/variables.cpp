class foo
{
    /// "isStatic" : true
    static int m;
    /// "isStatic" : false
    int b;
};

/// "internalContext" : { "localDeclarationCount" : 0}
void f(int);
/// "internalContext" : { "localDeclarationCount" : 1}
void f2(int a);
