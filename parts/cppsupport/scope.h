/*
 * file     : scope.h
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@htw-dresden.de
 * license  : gpl version >= 2
 *
 */

#ifndef _SCOPE_H
#define _SCOPE_H

/**
 * this class represents a scope-object like '1.2.1'
 */
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
    char cScope[ MAXSCOPES ];

};

#endif
