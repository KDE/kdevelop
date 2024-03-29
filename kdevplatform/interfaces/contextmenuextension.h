/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CONTEXTMENUEXTENSION_H
#define KDEVPLATFORM_CONTEXTMENUEXTENSION_H

#include <QString>
#include <QList>
#include <QScopedPointer>

#include "interfacesexport.h"

class QAction;
class QMenu;

namespace KDevelop
{
class ContextMenuExtensionPrivate;

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
    /** menu group to contain file analysis actions */
    static const QString AnalyzeFileGroup;
    /** menu group to contain project analysis actions */
    static const QString AnalyzeProjectGroup;
    /** The menu group containing navigation actions */
    static const QString NavigationGroup;

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
    const QScopedPointer<class ContextMenuExtensionPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ContextMenuExtension)
};

}

Q_DECLARE_TYPEINFO(KDevelop::ContextMenuExtension, Q_MOVABLE_TYPE);

#endif
