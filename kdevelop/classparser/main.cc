#include <iostream>
#include "ClassParser.h"

int main( int argc, char *argv[] )
{
  CClassParser cp;
  int start;
  
  start = ( strcmp( argv[ 1 ], "-store" ) == 0 ? 2 : 1 );
  for( int i=start; i < argc; i++ )
  {
    std::cout << "Parsing: [" << argv[ i ] << "]\n";
    cp.parse( argv[ i ] );
    if( start == 2 )
      cp.store.storeAll();
  }

  cp.out();
}
