/*
 * file     : scope.h
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@gmx.net
 * license  : gpl version >= 2
 */

#ifndef _SCOPE_H
#define _SCOPE_H

#include <qstring.h>

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
    CScope( const CScope& cs );

    /**
    * Init Constructor
    */
    CScope( const int iScope );

    /**
    * Destructor
    */
    ~CScope( );

    /**
    * operator =
    */
    const CScope& operator = ( const CScope& cs );

    /**
     * operator <
     */
    bool operator < ( const CScope& cs );

    /**
    * reset the scope value
    */
    void reset( );

    /**
    * to increase a scope
    */
    void increase( const int iScope );

    /**
     * to check the right scope for cc
     * wrapper function for operator < for easier use
     */
    bool isValidIn( const CScope& scope );

    /**
    * debug output
    */
    void debugOutput( ) const;

private:
    #define MAXSCOPES 50
    char cScope[ MAXSCOPES ];
};

#endif
