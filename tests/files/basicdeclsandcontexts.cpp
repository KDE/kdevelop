/**
 * "identifier" : "main",
 * "internalContext" : { "type" : "Function", "range" : "[(5, 0), (8, 1)]" },
 * "range" : "[(5, 4), (5, 8)]"
 */
int main()
{
    return 0;
}

/**
 * "identifier" : "foo",
 * "internalContext" : { "type" : "Class", "range" : "[(15, 0), (33, 1)]" },
 * "kind" : "Type"
 */
class foo
{
    /// "isStatic" : true
    static int m;
    /// "isStatic" : false
    int b;
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

/**
 * "identifier" : "bar",
 * "internalContext" : { "type" : "Namespace", "range" : "[(40, 0), (110, 1)]" },
 * "kind" : "Namespace"
 */
namespace bar
{
    /**
     * "identifier" : "foo",
     * "internalContext" : { "null" : false, "range" : "[(46, 4), (46, 14)]" }
     */
    void foo();

    /**
     * "identifier" : "foostruct",
     * "internalContext" : { "type" : "Class", "range" : "[(52, 4), (109, 5)]" }
     */
    struct foostruct
    {
        /**
         * "identifier" : "foostruct",
         * "internalContext" : { "type" : "Function", "range" : "[(58, 8), (60, 9)]" }
         */
        foostruct()
        {
        }

        /**
         * "identifier" : "~foostruct",
         * "internalContext" : { "type" : "Function", "range" : "[(66, 8), (68, 9)]" }
         */
        ~foostruct()
        {
        }

        /**
         * "identifier" : "operator int",
         * "internalContext" : { "type" : "Function", "range" : "[(74, 8), (77, 9)]" }
         */
        operator int()
        {
            return 0;
        }

        /**
         * "identifier" : "templateFunction",
         * "internalContext" : { "type" : "Function", "range" : "[(83, 8), (86, 9)]" }
         */
        template<class T>
        void templateFunction()
        {
        }

        /**
         * "identifier" : "fooenum",
         * "internalContext" : { "type" : "Enum", "range" : "[(92, 8), (94, 9)]" }
         */
        enum fooenum
        {
        };

        /**
         * "identifier" : "foomethod",
         * "internalContext" : { "type" : "Function", "range" : "[(100, 8), (100, 23)]" }
         */
        int foomethod();

        /**
         * "identifier" : "deffoomethod",
         * "internalContext" : { "type" : "Function", "range" : "[(106, 8), (108, 9)]" }
         */
        void deffoomethod()
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

/// "toString" : "struct myTemplateChild",
/// "kind" : "Type"
struct myTemplateChild : myTemplate<int> { };
