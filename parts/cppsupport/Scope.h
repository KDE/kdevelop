class CScope {

public:
    /**
    * Default Constructor
    */
    CScope( );

    /**
    * Copy Constructor
    */
    CScope( CScope& cs );

    /**
    * Init Constructor
    */
    CScope( int iScope );

    /**
    * Destructor
    */
    ~CScope( );

    /**
    * operator =
    */
    const CScope& operator = ( CScope& cs );

    /**
    * reset the scope value
    */
    void reset( );

    /**
    * to increase a scope
    */
    void increase( int iScope );

    /**
    * debug output
    */
    void debugOutput( );

private:
    #define MAXSCOPES 50

    struct myScope {
        char cScope[ MAXSCOPES ];
    } strScope;

};

