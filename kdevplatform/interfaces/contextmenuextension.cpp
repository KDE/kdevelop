/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contextmenuextension.h"

#include <QMap>

#include <QMenu>
#include <QAction>
#include <QActionGroup>

#include <KLocalizedString>

namespace KDevelop
{

const QString ContextMenuExtension::FileGroup     = QStringLiteral("FileGroup");
const QString ContextMenuExtension::RefactorGroup = QStringLiteral("RefactorGroup");
const QString ContextMenuExtension::BuildGroup    = QStringLiteral("BuildGroup");
const QString ContextMenuExtension::RunGroup      = QStringLiteral("RunGroup");
const QString ContextMenuExtension::DebugGroup    = QStringLiteral("DebugGroup");
const QString ContextMenuExtension::EditGroup     = QStringLiteral("EditGroup");
const QString ContextMenuExtension::VcsGroup      = QStringLiteral("VcsGroup");
const QString ContextMenuExtension::ProjectGroup  = QStringLiteral("ProjectGroup");
const QString ContextMenuExtension::OpenEmbeddedGroup  = QStringLiteral("OpenEmbeddedGroup");
const QString ContextMenuExtension::OpenExternalGroup  = QStringLiteral("OpenExternalGroup");
const QString ContextMenuExtension::AnalyzeFileGroup = QStringLiteral("AnalyzeFileGroup");
const QString ContextMenuExtension::AnalyzeProjectGroup = QStringLiteral("AnalyzeProjectGroup");
const QString ContextMenuExtension::NavigationGroup = QStringLiteral("NavigationGroup");
const QString ContextMenuExtension::ExtensionGroup  = QStringLiteral("ExtensionGroup");

class ContextMenuExtensionPrivate
{
public:
    QMap<QString,QList<QAction*> > extensions;
};

ContextMenuExtension::ContextMenuExtension()
    : d_ptr(new ContextMenuExtensionPrivate)
{
}

ContextMenuExtension::~ContextMenuExtension() = default;


ContextMenuExtension::ContextMenuExtension( const ContextMenuExtension& rhs )
    : d_ptr(new ContextMenuExtensionPrivate)
{
    Q_D(ContextMenuExtension);

    d->extensions = rhs.d_func()->extensions;
}

ContextMenuExtension& ContextMenuExtension::operator=( const ContextMenuExtension& rhs )
{
    Q_D(ContextMenuExtension);

    if( this == &rhs )
        return *this;

    d->extensions = rhs.d_func()->extensions;
    return *this;
}

QList<QAction*> ContextMenuExtension::actions( const QString& group ) const
{
    Q_D(const ContextMenuExtension);

    return d->extensions.value( group, QList<QAction*>() );
}

void ContextMenuExtension::addAction( const QString& group, QAction* action )
{
    Q_D(ContextMenuExtension);

    auto actionGroupIt = d->extensions.find(group);
    if (actionGroupIt == d->extensions.end()) {
        d->extensions.insert( group, QList<QAction*>() << action );
    } else
    {
        actionGroupIt->append(action);
    }
}

static
void populateMenuWithGroup(
    QMenu* menu,
    const QList<ContextMenuExtension>& extensions,
    const QString& groupName,
    const QString& groupDisplayName = QString(),
    const QString& groupIconName = QString(),
    bool forceAddMenu = false,
    bool addSeparator = true)
{
    QList<QAction*> groupActions;
    for (const ContextMenuExtension& extension : extensions) {
        groupActions += extension.actions(groupName);
    }
    // remove NULL QActions, if any. Those can end up in groupActions if plugins
    // like the debugger plugins are not loaded.
    groupActions.removeAll(nullptr);

    if (groupActions.isEmpty()) {
        return;
    }

    QMenu* groupMenu = menu;
    if ((groupActions.count() > 1 && !groupDisplayName.isEmpty()) ||
        (!groupDisplayName.isEmpty() && forceAddMenu)) {
        groupMenu = menu->addMenu(groupDisplayName);
        if (!groupIconName.isEmpty()) {
            groupMenu->setIcon(QIcon::fromTheme(groupIconName));
        }
    }

    for (QAction* action : std::as_const(groupActions)) {
        groupMenu->addAction(action);
    }

    if (addSeparator) {
        menu->addSeparator();
    }
}

void ContextMenuExtension::populateMenu(QMenu* menu, const QList<ContextMenuExtension>& extensions)
{
    populateMenuWithGroup(menu, extensions, BuildGroup);
    populateMenuWithGroup(menu, extensions, FileGroup);
    populateMenuWithGroup(menu, extensions, EditGroup);
    populateMenuWithGroup(menu, extensions, DebugGroup, i18nc("@title:menu", "Debug"));
    populateMenuWithGroup(menu, extensions, RefactorGroup, i18nc("@title:menu", "Refactor"));
    populateMenuWithGroup(menu, extensions, NavigationGroup);
    populateMenuWithGroup(menu, extensions, AnalyzeFileGroup, i18nc("@title:menu", "Analyze Current File With"), QStringLiteral("dialog-ok"), true, false);
    populateMenuWithGroup(menu, extensions, AnalyzeProjectGroup, i18nc("@title:menu", "Analyze Current Project With"), QStringLiteral("dialog-ok"), true);
    populateMenuWithGroup(menu, extensions, VcsGroup);
    populateMenuWithGroup(menu, extensions, ExtensionGroup);
}

}
