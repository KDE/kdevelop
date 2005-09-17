%{CPP_TEMPLATE}

#include <kmainwindow.h>
#include <klocale.h>

#include "%{APPNAMELC}.h"
#include "%{APPNAMELC}widget.h"

%{APPNAME}::%{APPNAME}()
    : KMainWindow( 0, "%{APPNAME}" )
{
    setCentralWidget( new %{APPNAME}Widget( this ) );
}

%{APPNAME}::~%{APPNAME}()
{
}

#include "%{APPNAMELC}.moc"
