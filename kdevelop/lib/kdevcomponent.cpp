#include "kdevcomponent.h"


KDevComponent::KDevComponent(QObject *parent, const char *name)
    : KParts::Plugin(parent, name)
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

void KDevComponent::compilationAborted()
{
}

void KDevComponent::projectOpened(CProject *prj)
{
}

void KDevComponent::projectClosed()
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

QWidget *KDevComponent::widget()
{
    return 0;
}
