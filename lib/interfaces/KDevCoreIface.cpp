#include <kdebug.h>
#include <dcopclient.h>
#include "KDevCoreIface.h"
#include "kdevcore.h"


KDevCoreIface::KDevCoreIface(KDevCore *core)
    : QObject(core), DCOPObject("KDevCore"), m_core(core)
{
    connect( m_core, SIGNAL(projectOpened()), this, SLOT(forwardProjectOpened()) );
    connect( m_core, SIGNAL(projectClosed()), this, SLOT(forwardProjectClosed()) );
}


KDevCoreIface::~KDevCoreIface()
{}


void KDevCoreIface::forwardProjectOpened()
{
    kdDebug(9000) << "dcop emitting project opened" << endl;
    emitDCOPSignal("projectOpened()", QByteArray());
}


void KDevCoreIface::forwardProjectClosed()
{
    kdDebug(9000) << "dcop emitting project closed" << endl;
    emitDCOPSignal("projectClosed()", QByteArray());
}


#include "KDevCoreIface.moc"
