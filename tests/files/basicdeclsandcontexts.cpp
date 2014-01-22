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
 * "internalContext" : { "type" : "Class" }
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
 * "internalContext" : { "type" : "Namespace" }
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

/// "toString" : "struct myStruct"
struct myStruct {};

/// "toString" : "class myClass"
class myClass {};

/// "toString" : "union myUnion"
union myUnion {};

/// "toString" : "myEnum myEnum"
enum myEnum {
    /// "toString" : "myEnum myEnumerator"
    myEnumerator
};

/// "toString" : "Import of bar"
using namespace bar;

/// "toString" : "Import bar as blub"
namespace blub = bar;

/// "toString" : "struct myTemplate"
template<class T>
struct myTemplate {};
