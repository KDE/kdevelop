%{CPP_TEMPLATE}

#include "%{APPNAMELC}.h"

#include <qlabel.h>

#include <kmainwindow.h>
#include <klocale.h>

%{APPNAME}::%{APPNAME}()
    : KMainWindow( 0, "%{APPNAME}" )
{
    // set the shell's ui resource file
    setXMLFile("%{APPNAMELC}ui.rc");

    new QLabel( "Hello World", this, "hello label" );
}

%{APPNAME}::~%{APPNAME}()
{
}

#include "%{APPNAMELC}.moc"
