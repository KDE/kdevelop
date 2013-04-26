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

#ifndef KDEVPLATFORM_CONTEXTMENUEXTENSION_H
#define KDEVPLATFORM_CONTEXTMENUEXTENSION_H

#include <QtCore/QString>
#include <QtCore/QList>

#include "interfacesexport.h"

class QAction;
class QMenu;
template <typename T> class QList;

namespace KDevelop
{
/**
 * For documentation on how to use this class, @see interfaces/context.h
 */
class KDEVPLATFORMINTERFACES_EXPORT ContextMenuExtension
{
public:

    /** The menu group containing file actions */
    static const QString FileGroup;
    /** menu group containing refactoring actions */
    static const QString RefactorGroup;
    /** menu group to contain build support actions */
    static const QString BuildGroup;
    /** menu group to contain run actions */
    static const QString RunGroup;
    /** menu group to contain debug actions */
    static const QString DebugGroup;
    /** menu group to contain editing actions */
    static const QString EditGroup;
    /** menu group to contain version control actions */
    static const QString VcsGroup;
    /** menu group to contain project actions */
    static const QString ProjectGroup;
    /** menu group to contain open in embedded editor actions */
    static const QString OpenEmbeddedGroup;
    /** menu group to contain open with external application actions */
    static const QString OpenExternalGroup;

    /** menu group that can contain any extension menu.
      * Actions for this extension will always be at the end
      * of the menu. Plugins using this should think about
      * providing a submenu, so the context menu doesn't get cluttered.
      */
    static const QString ExtensionGroup;

    /**
     * create new context menu extension object
     */
    ContextMenuExtension();
    ~ContextMenuExtension();
    ContextMenuExtension( const ContextMenuExtension& rhs );

    ContextMenuExtension& operator=( const ContextMenuExtension& rhs );

    /**
     * Add an action to the given menu group
     * @param group the menu group to which the action should be added
     * @param action the action to add to the menu group
     */
    void addAction( const QString& group, QAction* action );

    /**
     * Return all actions that are in the menu group
     * @param group the menu group from which to get the actions
     * @returns a list of actions for that menu group
     */
    QList<QAction*> actions( const QString& group ) const;

    /**
     * Populate a QMenu with the actions in the given context menu extensions.
     */
    static void populateMenu(QMenu* menu, const QList<ContextMenuExtension>& extensions);

private:
    class ContextMenuExtensionPrivate* const d;
};

}

#endif
