/***************************************************************************
                          componentmanager.h  -  interface to components
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


#ifndef _COMPONENTMANAGER_H_
#define _COMPONENTMANAGER_H_

#include <qlist.h>
#include <qstring.h>

class CProject;
class Component;


/**
 * The Component manager maintains a list of all components. It is a singleton
 * object that can be obtained by ComponentManager::self(). Components have to
 * be registered with registerComponent(). Messages can then be sent with the
 * methods named notify*. 
 */
class ComponentManager
{
public:
    static ComponentManager *self();
    void registerComponent(Component *component);
    
    void notifyDocPathChanged();
    void notifyCompilationAborted();
    void notifyProjectOpened(CProject *prj);
    void notifyProjectClosed();
    void notifyAddedFileToProject(const QString &name);
    void notifyRemovedFileFromProject(const QString &name);
    void notifySavedFile(const QString &name);

private:
    ComponentManager();
    QList<Component> components;
    static ComponentManager *manager;
};


#endif
