#include <qwidget.h>

#include <interfaces/interfaceinformationimp.h>

#include "%{APPNAMELC}iface.h"
#include "%{APPNAMELC}module.h"

%{APPNAME}Module::%{APPNAME}Module() {
    Interface* iface = new %{APPNAME}Interface( 0 );
    iface->setHardwareName( "vpn" );
    iface->setInterfaceName( "Test VPN" );
    m_interfaces.append( iface );

//  If we set up VPN via pptp
//  and networksettins was closed and now opened
//  we need to hide the ppp device behind us
//  One can do  this by calling setHandledInterfaceNames
//    setHandledInterfaceNames();
}

%{APPNAME}Module::~%{APPNAME}Module() {
    m_interfaces.setAutoDelete( true );
    m_interfaces.clear();
}


/*
 * We're a VPN module
 */
bool %{APPNAME}Module::isOwner(  Interface* iface ) {
 /* check if it is our device */
    return  m_interfaces.find(  iface ) != -1;
}

QWidget*  %{APPNAME}Module::configure( Interface* ) {
/* We don't have any Config for now */
    return 0l;
}

QWidget* %{APPNAME}Module::information(  Interface* iface ) {
    return  new InterfaceInformationImp(0, "Interface info", iface );
}

QList<Interface> %{APPNAME}Module::getInterfaces() {
    return m_interfaces;
}

void %{APPNAME}Module::possibleNewInterfaces( QMap<QString, QString>& map) {
    map.insert( QObject::tr("VPN PPTP"),
                QObject::tr("Add new Point to Point Tunnel Protocol connection" ) );
}


Interface* %{APPNAME}Module::addNewInterface( const QString& ) {
 /* check the str if we support more interfaces */
/*
    Interface* iface = new %{APPNAME}Interface( 0 );
    iface->setModuleOwner( this );
    return iface;*/

// if we would support saving interfaces we could add
// them here

    return 0;
}


bool %{APPNAME}Module::remove( Interface* ) {
/* we do not support removing our interface */
    return false;
}
