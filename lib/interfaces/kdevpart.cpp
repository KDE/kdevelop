#include "kdevpart.h"
#include "kdevpartcontroller.h"


#include "kdevapi.h"


KDevPart::KDevPart(KDevApi *api, QObject *parent, const char *name)
    : KParts::Part(parent, name), m_api(api)
{
}

KDevPart::~KDevPart()
{
}

KDevCore *KDevPart::core()
{
    return m_api->core;
}

KDevProject *KDevPart::project()
{
    return m_api->project;
}

ClassStore *KDevPart::classStore()
{
    return m_api->classStore;
}

/* added by daniel */
ClassStore*
KDevPart::ccClassStore( )
{
    return m_api->ccClassStore;
}

QDomDocument *KDevPart::projectDom()
{
    return m_api->projectDom;
}

KDevVersionControl *KDevPart::versionControl()
{
    return m_api->versionControl;
}

KDevLanguageSupport *KDevPart::languageSupport()
{
    return m_api->languageSupport;
}

KDevMakeFrontend *KDevPart::makeFrontend()
{
    return m_api->makeFrontend;
}

KDevAppFrontend *KDevPart::appFrontend()
{
    return m_api->appFrontend;
}

KDevPartController *KDevPart::partController()
{
    return m_api->getPartController();
}


void KDevPart::setWidget(QWidget*)
{
}

QWidget *KDevPart::widget()
{
  return 0;
}

#if 0
QValueList<KAction*> KDevPart::nodeActions(KDevNode*)
{
  return QValueList<KAction*>();
}
#endif

#include "kdevpart.moc"
