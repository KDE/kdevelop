/*
 * file     : scope.cpp
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@htw-dresden.de
 * license  : gpl version >= 2
 *
 */

#define ENABLEDEBUGOUTPUT
#include "dbg.h"

#include "scope.h"
#include <string.h>

CScope::CScope( )
{
    reset( );
}

CScope::CScope( int iScope )
{
    reset( );
    increase( iScope );
}

CScope::~CScope( )
{
}

CScope::CScope( CScope& cs )
{
    *this = cs;
}

const CScope&
CScope::operator = ( CScope& cs )
{
    memcpy( cScope, cs.cScope, sizeof( cs.cScope ) );

    return *this;
}

void
CScope::reset( )
{
    memset( cScope, 0, sizeof( cScope ) );
}

void
CScope::increase( int iScope )
{
    if( ( iScope - 1 ) <= MAXSCOPES ){
        if( cScope[ iScope - 1 ] + 1 ){
            cScope[ iScope - 1 ]++;
            cScope[ iScope ] = 0;
            return;
        }
        else {
            errln( "EE: CScope::increase overflow" );
        }
    }
    else {
        errln( "EE: CScope::increase iScope > MAXSCOPES" );
    }
}

void
CScope::debugOutput( )
{
    outln ( "*** debug output CScope started" );
    for( int i = 0; i < MAXSCOPES; i++ ){
        if( cScope[ i ] )
            err( static_cast <char> ( cScope[ i ] + '0' ) << "." );
        else
            break;
    }
    errln( "" );
    outln( "*** debug output CScope ended" );
}





