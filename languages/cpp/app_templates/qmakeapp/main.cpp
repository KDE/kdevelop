
#include <qapplication.h>
#include "%{APPNAMELC}.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    %{APPNAME} * mw = new %{APPNAME}();
    mw->setCaption( "%{APPNAME}" );
    mw->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
