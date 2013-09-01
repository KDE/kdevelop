/*
    This file is part of KDevelop

    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "projectfilterprovider.h"

#include <KPluginLoader>
#include <KPluginFactory>
#include <KAboutData>
#include <KSettings/Dispatcher>
#include <KIcon>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <QAction>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iuicontroller.h>

#include "projectfilterdebug.h"
#include <project/projectmodel.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(ProjectFilterProviderFactory, registerPlugin<ProjectFilterProvider>(); )
K_EXPORT_PLUGIN(ProjectFilterProviderFactory(
    KAboutData("kdevprojectfilter", "kdevprojectfilter", ki18n("Project Filter"),
               "0.1", ki18n("Configure which files and folders inside the project folder should be included or excluded."),
               KAboutData::License_GPL)))

ProjectFilterProvider::ProjectFilterProvider( QObject* parent, const QVariantList& /*args*/ )
    : IPlugin( ProjectFilterProviderFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( IProjectFilterProvider )

    connect(core()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
            SLOT(projectClosing(KDevelop::IProject*)));
    connect(core()->projectController(), SIGNAL(projectAboutToBeOpened(KDevelop::IProject*)),
            SLOT(projectAboutToBeOpened(KDevelop::IProject*)));

    updateProjectFilters();

    KSettings::Dispatcher::registerComponent(componentData(), this, "updateProjectFilters");
}

QSharedPointer<IProjectFilter> ProjectFilterProvider::createFilter(IProject* project) const
{
    return QSharedPointer<IProjectFilter>(new ProjectFilter(project, m_filters[project]));
}

ContextMenuExtension ProjectFilterProvider::contextMenuExtension(Context* context)
{
    ContextMenuExtension ret;
    if (!context->hasType(Context::ProjectItemContext)) {
        return ret;
    }

    ProjectItemContext* ctx = static_cast<ProjectItemContext*>( context );

    QList<ProjectBaseItem*> items = ctx->items();
    // filter out project roots, targets and items in targets
    QList< ProjectBaseItem* >::iterator it = items.begin();
    while (it != items.end()) {
        if ((*it)->isProjectRoot() || (*it)->target() || !(*it)->parent()->folder()) {
            it = items.erase(it);
        } else {
            ++it;
        }
    }
    if (items.isEmpty()) {
        return ret;
    }

    QAction* action = new QAction(KIcon("view-filter"),
                                  i18np("Exclude item from project.",
                                        "Exclude items from project",
                                        items.size()), this);
    action->setData(QVariant::fromValue(items));
    connect(action, SIGNAL(triggered(bool)), SLOT(addFilterFromContextMenu()));
    ret.addAction(ContextMenuExtension::FileGroup, action);
    return ret;
}

void ProjectFilterProvider::addFilterFromContextMenu()
{
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    QList<ProjectBaseItem*> items = action->data().value<QList<ProjectBaseItem*> >();
    QHash<IProject*, SerializedFilters> changedProjectFilters;
    foreach(ProjectBaseItem* item, items) {
        if (!changedProjectFilters.contains(item->project())) {
            changedProjectFilters[item->project()] = readFilters(item->project()->projectConfiguration());
        }
        SerializedFilters& filters = changedProjectFilters[item->project()];
        filters << SerializedFilter('/' + KUrl::relativeUrl(item->project()->folder(), item->url()),
                                    item->folder() ? Filter::Folders : Filter::Files);
    }
    QHash< IProject*, SerializedFilters >::const_iterator it = changedProjectFilters.constBegin();
    while (it != changedProjectFilters.constEnd()) {
        writeFilters(it.value(), it.key()->projectConfiguration());
        m_filters[it.key()] = deserialize(it.value());
        emit filterChanged(this, it.key());
        ++it;
    }

    KMessageBox::information(ICore::self()->uiController()->activeMainWindow(),
                             i18np("A filter for the item was added. To undo, use the project filter settings.",
                                   "A filter for the items was added. To undo, use the project filter settings.",
                                   items.size()), i18n("Project Filter Added"), "projectfilter-addfromctxmenu");
}

void ProjectFilterProvider::updateProjectFilters()
{
    foreach(IProject* project, core()->projectController()->projects()) {
        Filters newFilters = deserialize(readFilters(project->projectConfiguration()));
        Filters& filters = m_filters[project];
        if (filters != newFilters) {
            projectFilterDebug() << "project filter changed:" << project->name();
            filters = newFilters;
            emit filterChanged(this, project);
        }
    }
}

void ProjectFilterProvider::projectAboutToBeOpened(IProject* project)
{
    m_filters[project] = deserialize(readFilters(project->projectConfiguration()));
}

void ProjectFilterProvider::projectClosing(IProject* project)
{
    m_filters.remove(project);
}

#include "projectfilterprovider.moc"
