
#include "%{APPNAMELC}.h"

#include <qlabel.h>

#include <kmainwindow.h>
#include <klocale.h>

%{APPNAME}::%{APPNAME}()
    : KMainWindow( 0, "%{APPNAME}" )
{
    m_mainClass = new MainClass();
}

%{APPNAME}::~%{APPNAME}()
{
    if (m_mainClass) delete m_mainClass;
}

#include "%{APPNAMELC}.moc"
