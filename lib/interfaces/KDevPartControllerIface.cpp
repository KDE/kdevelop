#include <kdebug.h>
#include <dcopclient.h>
#include <kurl.h>


#include "KDevPartControllerIface.h"
#include "kdevpartcontroller.h"


KDevPartControllerIface::KDevPartControllerIface(KDevPartController *pc)
  : QObject(pc), DCOPObject("KDevPartController"), m_controller(pc)
{
  connect(pc, SIGNAL(loadedFile(const QString &)), this, SLOT(forwardLoadedFile(const QString &)));
  connect(pc, SIGNAL(savedFile(const QString &)), this, SLOT(forwardSavedFile(const QString &)));
}


KDevPartControllerIface::~KDevPartControllerIface()
{
}


void KDevPartControllerIface::editDocument(const QString &url, int lineNum)
{
  m_controller->editDocument(KURL(url), lineNum);
}


void KDevPartControllerIface::showDocument(const QString &url, const QString &context)
{
  m_controller->showDocument(KURL(url), context);
}


void KDevPartControllerIface::saveAllFiles()
{
  m_controller->saveAllFiles();
}


void KDevPartControllerIface::revertAllFiles()
{
  m_controller->revertAllFiles();
}


void KDevPartControllerIface::forwardLoadedFile(const QString &fileName)
{
  kdDebug(9000) << "dcop emitting loadedFile " << fileName << endl;
  emitDCOPSignal("projectOpened()", QByteArray());
}


void KDevPartControllerIface::forwardSavedFile(const QString &fileName)
{
  kdDebug(9000) << "dcop emitting savedFile " << fileName << endl;
  emitDCOPSignal("projectClosed()", QByteArray());
}


#include "KDevPartControllerIface.moc"
