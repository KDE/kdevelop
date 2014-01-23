/**
 * "identifier" : "main",
 * "internalContext" : { "type" : "Other" },
 * "range" : "[(5, 4), (5, 8)]"
 */
int main()
{
    return 0;
}

/**
 * "identifier" : "foo",
 * "internalContext" : { "type" : "Class" },
 * "kind" : "Type"
 */
class foo
{
    /// "EXPECT_FAIL" : { "isStatic" : "not a class-member-declaration yet" },
    /// "isStatic" : true
    static int m;
    /// "isStatic" : false
    int b;
    /// "EXPECT_FAIL" : { "isStatic" : "not a class-member-declaration yet", "isVirtual" : "not a class-function yet" },
    /// "isStatic" : false,
    /// "isVirtual" : true
    virtual void bla() {}
    /// "EXPECT_FAIL" : { "isVirtual" : "not a class-function yet" },
    /// "isStatic" : false,
    /// "isVirtual" : true
    virtual void bar();
    /// "EXPECT_FAIL" : { "isStatic" : "not a class-member-declaration yet" },
    /// "isStatic" : true,
    /// "isVirtual" : false
    static void bla2() {}
    /// "EXPECT_FAIL" : { "isStatic" : "not a class-member-declaration yet" },
    /// "isStatic" : true,
    /// "isVirtual" : false
    static void bar2();
};

/**
 * "identifier" : "bar",
 * "internalContext" : { "type" : "Namespace" },
 * "kind" : "Namespace"
 */
namespace bar
{
    /**
     * "identifier" : "foo",
     * "internalContext" : { "null" : true }
     */
    void foo();

    /**
     * "identifier" : "foostruct",
     * "internalContext" : { "type" : "Class" }
     */
    struct foostruct
    {
        /**
         * "identifier" : "fooenum",
         * "internalContext" : { "type" : "Enum" }
         */
        enum fooenum
        {
        };

        /**
         * "identifier" : "foomethod",
         * "internalContext" : { "null" : true }
         */
        int foomethod();

        /**
         * "identifier" : "deffoomethod",
         * "internalContext" : { "type" : "Other" }
         */
        int deffoomethod()
        {
        }
    };
}

/// "toString" : "struct myStruct",
/// "kind" : "Type"
struct myStruct {};

/// "toString" : "class myClass",
/// "kind" : "Type"
class myClass {};

/// "toString" : "union myUnion",
/// "kind" : "Type"
union myUnion {};

/// "toString" : "myEnum myEnum",
/// "kind" : "Type"
enum myEnum {
    /// "toString" : "myEnum myEnumerator",
    /// "kind" : "Type"
    myEnumerator
};

/// "toString" : "Import of bar",
/// "kind" : "NamespaceAlias"
using namespace bar;

/// "toString" : "Import bar as blub",
/// "kind" : "NamespaceAlias"
namespace blub = bar;

/// "toString" : "struct myTemplate",
/// "kind" : "Type"
template<class T>
struct myTemplate {};
