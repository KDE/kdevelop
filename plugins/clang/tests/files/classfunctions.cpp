struct foo
{
    /// "isDefinition": true
    foo() {}
    /// "isDefinition": true
    ~foo() {}
    /// "isDefinition": true
    void blub() {}
};

struct bar
{
    /// "isDefinition": false
    bar();
    /// "isDefinition": false
    ~bar();
    /// "isDefinition": false
    void blub();
};
