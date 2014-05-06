/**
 * "identifier" : "main",
 * "internalContext" : { "type" : "Function" },
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
 * "internalContext" : { "type" : "Namespace" },
 * "kind" : "Namespace"
 */
namespace bar
{
    /**
     * "identifier" : "foo",
     * "internalContext" : { "null" : false }
     */
    void foo();

    /**
     * "identifier" : "foostruct",
     * "internalContext" : { "type" : "Class" }
     */
    struct foostruct
    {
        /**
         * "identifier" : "foostruct",
         * "internalContext" : { "type" : "Function" }
         */
        foostruct()
        {
        }

        /**
         * "identifier" : "~foostruct",
         * "internalContext" : { "type" : "Function" }
         */
        ~foostruct()
        {
        }

        /**
         * "identifier" : "operator int",
         * "internalContext" : { "type" : "Function" }
         */
        operator int()
        {
            return 0;
        }

        /**
         * "identifier" : "templateFunction",
         * "internalContext" : { "type" : "Function" }
         */
        template<class T>
        void templateFunction()
        {
        }

        /**
         * "identifier" : "fooenum",
         * "internalContext" : { "type" : "Enum" }
         */
        enum fooenum
        {
        };

        /**
         * "identifier" : "foomethod",
         * "internalContext" : { "null" : false, "type" : "Function" }
         */
        int foomethod();

        /**
         * "identifier" : "deffoomethod",
         * "internalContext" : { "type" : "Function" }
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
