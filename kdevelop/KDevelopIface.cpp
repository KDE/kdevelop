#include <kaction.h>
#include <kdcopactionproxy.h>
#include <kapp.h>
#include <dcopclient.h>
#include "KDevelopIface.h"
#include "kdevelopcore.h"


KDevelopIface::KDevelopIface(KDevelopCore *core)
    : DCOPObject("KDevelop")
{
    m_core = core;
    m_actionProxy = new KDCOPActionProxy(core->actionCollection(), this);
}


KDevelopIface::~KDevelopIface()
{
    delete m_actionProxy;
}


bool KDevelopIface::openProjectSpace(const QString &fileName)
{
    return m_core->openProjectSpace(fileName);
}


void KDevelopIface::closeProjectSpace()
{
    m_core->closeProjectSpace();
}


void KDevelopIface::changeProjectSpace ()
{
//    m_core->changeProjectSpace();
}


QCStringList KDevelopIface::actions()
{
    QCStringList list;
    const QValueList<KAction*> &actions = m_actionProxy->actions();
    QValueList<KAction*>::ConstIterator it;
    for (it = actions.begin(); it != actions.end(); ++it)
        list.append((*it)->name());

    return list;
}


DCOPRef KDevelopIface::action(const QCString &name)
{
    return DCOPRef(kapp->dcopClient()->appId(), m_actionProxy->actionObjectId(name));
}
