#include <iostream.h>
#include "ClassParser.h"

int main( int argc, char *argv[] )
{
  CClassParser cp;
  
  for( int i=1; i < argc; i++ )
  {
    cout << "Parsing: " << argv[ i ] << "\n";
    cp.parse( argv[ i ] );
  }

  cp.out();
}
