/*
 * file     : scope.cpp
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@gmx.net
 * license  : gpl version >= 2
 */

#define ENABLEDEBUGOUTPUT
#include "dbg.h"
#include <kdebug.h>
#include "scope.h"
#include <string.h>

CScope::CScope( )
{
    reset( );
}

CScope::CScope( const int iScope )
{
    reset( );
    increase( iScope );
}

CScope::~CScope( )
{
}

CScope::CScope( const CScope& cs )
{
    *this = cs;
}

const CScope&
CScope::operator = ( const CScope& cs )
{
    memcpy( cScope, cs.cScope, sizeof( cs.cScope ) );

    return *this;
}

inline bool
CScope::operator < ( const CScope& cs )
{
    /*
     * a short description how that algorithm works
     * imagine the following method with these scopes:
     * {       1
     *   {     1.1
     *   }
     *   {     1.2
     *     {   1.2.1
     *     }
     *   }
     *   {     1.3
     *    {    1.3.1
     *     {   1.3.1.1
     *
     * valid in all scopes are variables from scope 1
     * valid in a given scope are variables from all (unclosed) scopes above
     * comparing scope 1 and 1.3 means: 1 == 1 and a not existing second number in 1 -> true
     * comparing scope 1.2.1 and 1.3.1 : 1 == 1, 2 != 3 -> false
     *
     */

    for( int i = 0; i < MAXSCOPES; i++ ){
        if( ( cs.cScope[ i ] == cScope[ i ] ) && cScope[ i ] == 0 )
            return true;
        if( !cs.cScope[ i ] )
            return false;
        if( !cScope[ i ] )
            return true;
        if( cScope[ i ] != cs.cScope[ i ] )
            return false;
    }
    return true;
}

void
CScope::reset( )
{
    memset( cScope, 0, sizeof( cScope ) );
}

void
CScope::increase( const int iScope )
{
    if( ( iScope - 1 ) <= MAXSCOPES ){
        if( cScope[ iScope - 1 ] + 1 ){
            cScope[ iScope - 1 ]++;
            cScope[ iScope ] = 0;
            return;
        }
        else {
            kdDebug(9007) << "EE: CScope::increase overflow" << endl;
        }
    }
    else {
        kdDebug(9007) << "EE: CScope::increase iScope > MAXSCOPES" << endl;
    }
}

void
CScope::decrease( const int iScope )
{
    if( iScope > 0 ){
        if( cScope[ iScope ] - 1 ){
            cScope[ iScope ]--;
            cScope[ iScope + 1 ] = 0;
            return;
        }
        else {
            cScope[ iScope ] = 0;
        }
    }
    else {
        kdDebug(9007) << "EE: CScope::decrease iScope < 0" << endl;
    }
}

bool
CScope::isValidIn( const CScope& scope )
{
    return true ? *this < scope : false;
}

void
CScope::debugOutput( ) const
{
    outln ( "*** debug output CScope started" );
    for( int i = 0; i < MAXSCOPES; i++ ){
        if( cScope[ i ] )
            errmsg( static_cast <char> ( cScope[ i ] + '0' ) << "." );
        else
            break;
    }
//    errln( "" );
    outln( "*** debug output CScope ended" );
}
