/***************************************************************************
                          componentmanager.cpp  -  interface to components
                             -------------------                                         
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "componentmanager.h"
#include "component.h"


ComponentManager *ComponentManager::manager = 0;


ComponentManager::ComponentManager()
{}


ComponentManager *ComponentManager::self()
{
    if (manager == 0)
        manager = new ComponentManager();
    return manager;
}


void ComponentManager::registerComponent(Component *component)
{
    components.append(component);
}


void ComponentManager::createConfigWidgets(CustomizeDialog *parent)
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->createConfigWidget(parent);
}


void ComponentManager::notifyDocPathChanged()
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->docPathChanged();
}


void ComponentManager::notifyCompilationAborted()
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->compilationAborted();
}


void ComponentManager::notifyProjectOpened(CProject *prj)
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->projectOpened(prj);
}


void ComponentManager::notifyProjectClosed()
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->projectClosed();
}


void ComponentManager::notifyAddedFileToProject(const QString &name)
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->addedFileToProject(name);
}
        

void ComponentManager::notifyRemovedFileFromProject(const QString &name)
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->removedFileFromProject(name);
}


void ComponentManager::notifySavedFile(const QString &name)
{
    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->savedFile(name);
}
