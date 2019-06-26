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

#include <KPluginFactory>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <KLocalizedString>

#include <QAction>
#include <QIcon>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iuicontroller.h>

#include <debug.h>
#include "projectfilterconfigpage.h"
#include <project/projectmodel.h>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ProjectFilterProviderFactory, "kdevprojectfilter.json", registerPlugin<ProjectFilterProvider>();)

ProjectFilterProvider::ProjectFilterProvider( QObject* parent, const QVariantList& /*args*/ )
    : IPlugin( QStringLiteral( "kdevprojectfilter" ), parent )
{
    connect(core()->projectController(), &IProjectController::projectClosing,
            this, &ProjectFilterProvider::projectClosing);
    connect(core()->projectController(), &IProjectController::projectAboutToBeOpened,
            this, &ProjectFilterProvider::projectAboutToBeOpened);

    // initialize the filters for each project
    const auto projects = core()->projectController()->projects();
    for (IProject* project : projects) {
        updateProjectFilters(project);
    }
}

QSharedPointer<IProjectFilter> ProjectFilterProvider::createFilter(IProject* project) const
{
    return QSharedPointer<IProjectFilter>(new ProjectFilter(project, m_filters[project]));
}

ContextMenuExtension ProjectFilterProvider::contextMenuExtension(Context* context, QWidget* parent)
{
    ContextMenuExtension ret;
    if (!context->hasType(Context::ProjectItemContext)) {
        return ret;
    }

    auto* ctx = static_cast<ProjectItemContext*>( context );

    QList<ProjectBaseItem*> items = ctx->items();
    // filter out project roots and items in targets
    QList< ProjectBaseItem* >::iterator it = items.begin();
    while (it != items.end()) {
        if ((*it)->isProjectRoot() || !(*it)->parent()->folder()) {
            it = items.erase(it);
        } else {
            ++it;
        }
    }
    if (items.isEmpty()) {
        return ret;
    }

    QAction* action = new QAction(QIcon::fromTheme(QStringLiteral("view-filter")),
                                  i18np("Exclude Item From Project",
                                        "Exclude Items From Project",
                                        items.size()), parent);
    action->setData(QVariant::fromValue(items));
    connect(action, &QAction::triggered, this, &ProjectFilterProvider::addFilterFromContextMenu);
    ret.addAction(ContextMenuExtension::FileGroup, action);
    return ret;
}

void ProjectFilterProvider::addFilterFromContextMenu()
{
    auto* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    const QList<ProjectBaseItem*> items = action->data().value<QList<ProjectBaseItem*>>();
    QHash<IProject*, SerializedFilters> changedProjectFilters;
    for (ProjectBaseItem* item : items) {
        auto filterIt = changedProjectFilters.find(item->project());
        if (filterIt == changedProjectFilters.end()) {
            filterIt = changedProjectFilters.insert(item->project(), readFilters(item->project()->projectConfiguration()));
        }
        SerializedFilters& filters = *filterIt;
        Path path;
        if (item->target()) {
            path = Path(item->parent()->path(), item->text());
        } else {
            path = item->path();
        }
        filters << SerializedFilter(QLatin1Char('/') + item->project()->path().relativePath(path),
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
                                   items.size()), i18n("Project Filter Added"), QStringLiteral("projectfilter-addfromctxmenu"));
}

void ProjectFilterProvider::updateProjectFilters(IProject* project)
{
    Filters newFilters = deserialize(readFilters(project->projectConfiguration()));
    Filters& filters = m_filters[project];
    if (filters != newFilters) {
        qCDebug(PLUGIN_PROJECTFILTER) << "project filter changed:" << project->name();
        filters = newFilters;
        emit filterChanged(this, project);
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

int ProjectFilterProvider::perProjectConfigPages() const
{
    return 1;
}

ConfigPage* ProjectFilterProvider::perProjectConfigPage(int i, const ProjectConfigOptions& options, QWidget* parent)
{
    return i == 0 ? new ProjectFilterConfigPage(this, options, parent) : nullptr;
}

#include "projectfilterprovider.moc"
