#include "dbgmanager.h"

#include "dbgfactory.h"

#include "kdevcomponent.h"

DbgManager::DbgManager(QObject *parent, const char *name ) :
  KDevComponent(parent, name)
{
  setInstance(DbgFactory::instance());
  setXMLFile("kdevelopdbg.rc" );
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
