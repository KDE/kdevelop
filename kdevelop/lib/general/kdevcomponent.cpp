#include "kdevcomponent.h"
#include "kdevapi.h"


KDevComponent::KDevComponent(QObject *parent, const char *name)
    : KParts::Part(parent, name)
{
}

KDevComponent::~KDevComponent()
{
}

KDevelopCoreBase *KDevComponent::TheCore()
{
    return m_pCore;
}

ProjectSpace *KDevComponent::projectSpace()
{
    return m_api->projectSpace;
}

ClassStore *KDevComponent::classStore()
{
    return m_api->classStore;
}

KDevVersionControl *KDevComponent::versionControl()
{
    return m_api->versionControl;
}

KDevLanguageSupport *KDevComponent::languageSupport()
{
    return m_api->languageSupport;
}

KDevEditorManager *KDevComponent::editorManager()
{
    return m_api->editorManager;
}

KDevMakeFrontend *KDevComponent::makeFrontend()
{
    return m_api->makeFrontend;
}

KDevAppFrontend *KDevComponent::appFrontend()
{
    return m_api->appFrontend;
}

void KDevComponent::setupInternal(KDevelopCoreBase * pCore, KDevApi *api)
{
    // Give them direct access to the KDevelop core.
    m_pCore = pCore;

    m_api = api;
}

void KDevComponent::setupGUI()
{
}

void KDevComponent::configWidgetRequested(KDialogBase*)
{
}

void KDevComponent::stopButtonClicked()
{
}

void KDevComponent::projectSpaceOpened()
{
}

void KDevComponent::setWidget(QWidget*)
{
}

QWidget *KDevComponent::widget()
{
  return 0;
}

KAboutData* KDevComponent::aboutPlugin(){
  return 0;
}

QList<KAction> KDevComponent::kdevNodeActions(KDevNode*){
  return QList<KAction>();
}

#include "kdevcomponent.moc"
