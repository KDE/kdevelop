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

#include "projectfiltermanager.h"

#include <QVector>

#include <KDebug>

#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

#include "interfaces/iprojectfilterprovider.h"
#include "interfaces/iprojectfilter.h"
#include "interfaces/iprojectfilemanager.h"

using namespace KDevelop;

//BEGIN helper
namespace {

struct Filter
{
    QSharedPointer<IProjectFilter> filter;
    // required for bookkeeping
    IProjectFilterProvider* provider;
};

}

Q_DECLARE_TYPEINFO(Filter, Q_MOVABLE_TYPE);

//END helper

//BEGIN private
struct ProjectFilterManager::Private
{
    void pluginLoaded(IPlugin* plugin);
    void unloadingPlugin(IPlugin* plugin);
    void filterChanged(IProjectFilterProvider* provider, IProject* project);

    QVector<IProjectFilterProvider*> m_filterProvider;
    QHash<IProject*, QVector<Filter> > m_filters;

    ProjectFilterManager* q;
};

void ProjectFilterManager::Private::pluginLoaded(IPlugin* plugin)
{
    IProjectFilterProvider* filterProvider = plugin->extension<IProjectFilterProvider>();
    if (filterProvider) {
        m_filterProvider << filterProvider;
        QObject::connect(plugin, SIGNAL(filterChanged(KDevelop::IProjectFilterProvider*, KDevelop::IProject*)),
                         q, SLOT(filterChanged(KDevelop::IProjectFilterProvider*, KDevelop::IProject*)));
        QHash< IProject*, QVector< Filter > >::iterator it = m_filters.begin();
        while(it != m_filters.end()) {
            Filter filter;
            filter.provider = filterProvider;
            filter.filter = filterProvider->createFilter(it.key());
            it.value().append(filter);
            ++it;
        }
    }
}

void ProjectFilterManager::Private::unloadingPlugin(IPlugin* plugin)
{
    IProjectFilterProvider* filterProvider = plugin->extension<IProjectFilterProvider>();
    if (filterProvider) {
        int idx = m_filterProvider.indexOf(qobject_cast<IProjectFilterProvider*>(plugin));
        Q_ASSERT(idx != -1);
        m_filterProvider.remove(idx);
        QHash< IProject*, QVector<Filter> >::iterator filtersIt = m_filters.begin();
        while(filtersIt != m_filters.end()) {
            QVector<Filter>& filters = filtersIt.value();
            QVector<Filter>::iterator filter = filters.begin();
            while(filter != filters.end()) {
                if ((*filter).provider == filterProvider) {
                    filter = filters.erase(filter);
                    continue;
                }
                ++filter;
            }
            ++filtersIt;
        }
    }
}

void ProjectFilterManager::Private::filterChanged(IProjectFilterProvider* provider, IProject* project)
{
    if (!m_filters.contains(project)) {
        return;
    }

    QVector< Filter >& filters = m_filters[project];
    QVector<Filter>::iterator it = filters.begin();
    while(it != filters.end()) {
        if (it->provider == provider) {
            it->filter = provider->createFilter(project);
            kDebug(9517) << "project filter changed, reloading" << project->name();
            project->projectFileManager()->reload(project->projectItem());
            return;
        }
        ++it;
    }
    Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown provider changed its filter");
}
//END private

//BEGIN
ProjectFilterManager::ProjectFilterManager(QObject* parent)
    : QObject(parent)
    , d(new ProjectFilterManager::Private)
{
    d->q = this;

    connect(ICore::self()->pluginController(), SIGNAL(pluginLoaded(KDevelop::IPlugin*)),
            SLOT(pluginLoaded(KDevelop::IPlugin*)));
    connect(ICore::self()->pluginController(), SIGNAL(unloadingPlugin(KDevelop::IPlugin*)),
            SLOT(unloadingPlugin(KDevelop::IPlugin*)));

    foreach(IPlugin* plugin, ICore::self()->pluginController()->loadedPlugins()) {
        d->pluginLoaded(plugin);
    }
}

ProjectFilterManager::~ProjectFilterManager()
{

}

void ProjectFilterManager::add(IProject* project)
{
    QVector<Filter> filters;
    filters.reserve(d->m_filterProvider.size());
    foreach(IProjectFilterProvider* provider, d->m_filterProvider) {
        Filter filter;
        filter.provider = provider;
        filter.filter = provider->createFilter(project);
        filters << filter;
    }
    d->m_filters[project] = filters;
}

void ProjectFilterManager::remove(IProject* project)
{
    d->m_filters.remove(project);
}

bool ProjectFilterManager::isManaged(IProject* project) const
{
    return d->m_filters.contains(project);
}

bool ProjectFilterManager::isValid(const KUrl& path, bool isFolder, IProject* project) const
{
    foreach(const Filter& filter, d->m_filters[project]) {
        if (!filter.filter->isValid(path, isFolder)) {
            return false;
        }
    }
    return true;
}

QVector< QSharedPointer< IProjectFilter > > ProjectFilterManager::filtersForProject(IProject* project) const
{
    QVector< QSharedPointer< IProjectFilter > > ret;
    QVector< Filter > filters = d->m_filters[project];
    ret.reserve(filters.size());
    foreach(const Filter& filter, filters) {
        ret << filter.filter;
    }
    return ret;
}
//END

#include "projectfiltermanager.moc"
