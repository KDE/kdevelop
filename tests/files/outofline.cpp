class foo
{
    /// "qualifiedIdentifier": "foo::bar",
    /// "isDefinition": false,
    /// "definition": {
    ///     "range": "[(11, 9), (11, 12)]",
    ///     "qualifiedIdentifier": "foo::bar", "EXPECT_FAIL": {"qualifiedIdentifier": "the parent context is not set properly"}
    /// }
    int bar();
};

int foo::bar() {}
