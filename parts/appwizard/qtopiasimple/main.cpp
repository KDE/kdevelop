
#include <qtopia/qpeapplication.h>

int main( int argc, char **argv )
{
    QPEApplication a( argc, argv );

    QWidget m;
    a.showMainWidget( &m );

    return a.exec();
}

