#include "Scope.h"
#include <iostream.h>
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
    memcpy( strScope.cScope, cs.strScope.cScope, sizeof( char ) * MAXSCOPES );

    return *this;
}

void
CScope::reset( )
{
    memset( strScope.cScope, 0, sizeof( char ) * MAXSCOPES );
}

void
CScope::increase( int iScope )
{
    if( ( iScope - 1 ) <= MAXSCOPES ){
        if( strScope.cScope[ iScope - 1 ] + 1 ){
            strScope.cScope[ iScope - 1 ]++;
            strScope.cScope[ iScope ] = 0;
        }
        else {
            cerr << "EE: CScope::increase overflow" << endl;
        }
    }
    else {
        cerr << "EE: CScope::increase iScope > MAXSCOPES" << endl;
    }
}

void
CScope::debugOutput( )
{
//    cout << "*** debug output CScope started" << endl;
    for( int i = 0; i < MAXSCOPES; i++ ){
        if( strScope.cScope[ i ] )
            cerr << static_cast <char> ( strScope.cScope[ i ] + '0' ) << ".";
        else
            break;
    }
//    cout << "*** debug output CScope ended" << endl;
}





