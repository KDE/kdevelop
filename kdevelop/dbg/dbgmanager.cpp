#include "dbgmanager.h"

#include "kdevcomponent.h"

DbgManager::DbgManager(QObject *parent, const char *name ) :
  KDevComponent(parent, name)
{
}

DbgManager::~DbgManager()
{
}

void DbgManager::setupGUI()
{}

void DbgManager::compilationAborted()
{}

void DbgManager::projectClosed()
{}

void DbgManager::projectOpened(CProject *prj)
{}
