/**
 * "identifier" : "main",
 * "internalContext" : { "type" : "Other" }
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

};

/**
 * "identifier" : "bar",
 * "internalContext" : { "type" : "Namespace" }
 */
namespace bar
{
    /**
     * "identifier" : "foo"
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
         * "identifier" : "foomethod"
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
