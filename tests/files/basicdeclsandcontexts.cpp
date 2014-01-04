/**
 * "identifier" : "main",
 * "internalContext" : { "type" : "Other" },
 * "range" : "[(5, 4), (5, 8)]"
 */
int main()
{
    /// "toString" : "int a"
    int a = 0;
    return 0;
}

/**
 * "identifier" : "foo",
 * "internalContext" : { "type" : "Class" }
 */
class foo
{

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
