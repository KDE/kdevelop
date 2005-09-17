
#include "%{APPNAMELC}.h"
#include <qpe/qpeapplication.h>

int main( int argc, char ** argv )
{
    QPEApplication a( argc, argv );

    %{APPNAME} mw;
    a.showMainWidget( &mw );

    return a.exec();
}
