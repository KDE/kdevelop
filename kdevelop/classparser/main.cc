#include <qlist.h>
#include <iostream.h>
#include "ClassParser.h"
#include "ParsedClass.h"

int main( int argc, char *argv[] )
{
  CClassParser cp;
  
  for( int i=1; i < argc; i++ )
    cp.parse( argv[ i ] );

  cp.out();
}
