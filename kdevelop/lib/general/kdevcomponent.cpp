#include "kdevcomponent.h"



KDevComponent::KDevComponent(QObject *parent, const char *name)
    : KParts::Part(parent, name)
{
}

KDevComponent::~KDevComponent()
{
}

void KDevComponent::setupGUI()
{
}

void KDevComponent::configWidgetRequested(KDialogBase *dlg)
{
}

void KDevComponent::commandRequested(const QString &command)
{
}

void KDevComponent::stopButtonClicked()
{
}

void KDevComponent::projectOpened(CProject *prj)
{
}

void KDevComponent::projectClosed()
{
}

void KDevComponent::versionControlOpened(KDevVersionControl *vc)
{
}

void KDevComponent::versionControlClosed()
{
}

void KDevComponent::languageSupportOpened(KDevLanguageSupport *ls)
{
}

void KDevComponent::languageSupportClosed()
{
}


void KDevComponent::classStoreOpened(ClassStore *store)
{
}

void KDevComponent::classStoreClosed()
{
}

void KDevComponent::addedFileToProject(const QString&)
{
}

void KDevComponent::removedFileFromProject(const QString&)
{
}

void KDevComponent::savedFile(const QString&)
{
}


void KDevComponent::setWidget(QWidget *w)
{
}

QWidget *KDevComponent::widget()
{
    return 0;
}
KAboutData* KDevComponent::aboutPlugin(){
  return 0;
}
