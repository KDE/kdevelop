#include "kdevcomponent.h"


KDevComponent::KDevComponent(QObject *parent, const char *name)
    : KParts::Part(parent, name)
{
}

KDevComponent::~KDevComponent()
{
}

void KDevComponent::createConfigWidget(CustomizeDialog *parent)
{
}

void KDevComponent::docPathChanged()
{
}

void KDevComponent::compilationStarted(const QString &command)
{
}

void KDevComponent::compilationAborted()
{
}

void KDevComponent::projectOpened(CProject *prj)
{
}

void KDevComponent::projectClosed()
{
}

void KDevComponent::classStoreOpened(CClassStore *store)
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
