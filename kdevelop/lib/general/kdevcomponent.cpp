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

void KDevComponent::setupGUI()
{
}

void KDevComponent::configWidgetRequested(KDialogBase*)
{
}

void KDevComponent::commandRequested(const QString&)
{
}

void KDevComponent::stopButtonClicked()
{
}

void KDevComponent::projectSpaceOpened(ProjectSpace*)
{
}

void KDevComponent::projectSpaceClosed()
{
}

void KDevComponent::versionControlOpened(KDevVersionControl*)
{
}

void KDevComponent::versionControlClosed()
{
}

void KDevComponent::languageSupportOpened(KDevLanguageSupport*)
{
}

void KDevComponent::languageSupportClosed()
{
}


void KDevComponent::classStoreOpened(ClassStore*)
{
}

void KDevComponent::classStoreClosed()
{
}

void KDevComponent::addedFileToProject(KDevFileNode*)
{
}

void KDevComponent::removedFileFromProject(KDevFileNode*)
{
}

void KDevComponent::savedFile(const QString&)
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
void KDevComponent::setKDevNodeActions(QList<KAction>*){
}
QList<KAction>* KDevComponent::kdevNodeActions(KDevNode*){
  return 0;
}
void KDevComponent::writeProjectSpaceGlobalConfig(QDomDocument&){
}
void KDevComponent::writeProjectSpaceUserConfig(QDomDocument&){
}
void KDevComponent::readProjectSpaceGlobalConfig(QDomDocument&){
}
void KDevComponent::readProjectSpaceUserConfig(QDomDocument&){
}

