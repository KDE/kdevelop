#include "kdevcomponent.h"
#include <qwidget.h>
#include <qpopupmenu.h>
#include <iostream.h>



KDevComponent::KDevComponent(QObject *parent, const char *name)
    : KParts::Part(parent, name)
{
}

KDevComponent::~KDevComponent()
{
}

ProjectSpace *KDevComponent::projectSpace()
{
    return m_projectSpace;
}

ClassStore *KDevComponent::classStore()
{
    return m_classStore;
}

KDevVersionControl *KDevComponent::versionControl()
{
    return m_versionControl;
}

KDevLanguageSupport *KDevComponent::languageSupport()
{
    return m_languageSupport;
}

KDevEditorManager *KDevComponent::editorManager()
{
    return m_editorManager;
}

KDevMakeFrontend *KDevComponent::makeFrontend()
{
    return m_makeFrontend;
}

KDevAppFrontend *KDevComponent::appFrontend()
{
    return m_appFrontend;
}

void KDevComponent::setupInternal(ProjectSpace *ps, ClassStore *cs,
                                  KDevVersionControl *vc, KDevLanguageSupport *ls,
                                  KDevEditorManager *em, KDevMakeFrontend *mf,
                                  KDevAppFrontend *af)
{
    m_projectSpace = ps;
    m_classStore = cs;
    m_versionControl = vc;
    m_languageSupport = ls;
    m_editorManager = em;
    m_makeFrontend = mf;
    m_appFrontend = af;
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

void KDevComponent::versionControlOpened()
{
}

void KDevComponent::languageSupportOpened()
{
}

void KDevComponent::editorManagerOpened()
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
