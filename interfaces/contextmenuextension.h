/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef CONTEXTMENUEXTENSION_H
#define CONTEXTMENUEXTENSION_H

#include "interfacesexport.h"

class KAction;
template <typename T> class QList;
namespace KDevelop
{

class KDEVPLATFORMINTERFACES_EXPORT ContextMenuExtension
{
public:
    enum Group {
        FileGroup = 0     /**< The menu group containing file actions */,
        RefactorGroup = 1 /**< menu group containing refactoring actions */,
        BuildGroup = 2    /**< menu group to contain build support actions */,
        RunGroup = 3      /**< menu group to contain run actions */,
        DebugGroup = 4    /**< menu group to contain debug actions */,
        EditGroup = 5     /**< menu group to contain editing actions */,
        VcsGroup = 6      /**< menu group to contain Vcs actions */,
        ProjectGroup = 7  /**< menu group to contain project actions */,
        UserGroup = 1000  /**< starting point for plugins to define their own groups */
    };

    /**
     * create new context menu extension object
     */
    ContextMenuExtension();
    ~ContextMenuExtension();
    ContextMenuExtension( const ContextMenuExtension& rhs );

    ContextMenuExtension& operator=( const ContextMenuExtension& rhs );
    
    /**
     * add an action to the given menu group
     * @param group the menu group to which the action should be added
     * @param action the action to add to the menu group
     */
    void addAction( Group group, KAction* action );

    /**
     * return all actions that are in the menu group
     * @param group the menu group from which to get the actions
     * @returns a list of actions for that menu group
     */
    QList<KAction*> actions( Group group );

private:
    class ContextMenuExtensionPrivate* const d;
};

}

#endif
