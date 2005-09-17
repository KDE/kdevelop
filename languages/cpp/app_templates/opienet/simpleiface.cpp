#include "%{APPNAMELC}iface.h"

%{APPNAME}Interface::%{APPNAME}Interface( QObject* parent,
                                    const char* name,
                                    bool status )
    : Interface(parent, name, status )
{
}

%{APPNAME}Interface::~%{APPNAME}Interface() {
}

bool %{APPNAME}Interface::refresh() {
/* we do VPN over ppp
 * so replace the interfaceName with
 * something actual existing
 * I take wlan0 in my case
 */
    QString old = getInterfaceName();
    qWarning("Interface name was " + old );
    setInterfaceName( "wlan0" );

    bool b =Interface::refresh();
    setInterfaceName( old );

/*  new and old interface name */
    emit updateInterface(this);
    return b;
}


void %{APPNAME}Interface::start() {
// call pptp
    setStatus(true);
    refresh();
    emit updateMessage("VPN started");
}

void %{APPNAME}Interface::stop() {
    setStatus(false );
    refresh();
    emit updateMessage("VPN halted");
}
