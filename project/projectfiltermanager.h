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

#ifndef KDEVPLATFORM_PROJECTFILTERMANAGER_H
#define KDEVPLATFORM_PROJECTFILTERMANAGER_H

#include "projectexport.h"

#include <QObject>
#include <QVector>
#include <QSharedPointer>

namespace KDevelop {

class IProjectFilter;
class IProjectFilterProvider;
class IPlugin;
class IProject;
class Path;

/**
 * @short A helper class to manage project filtering in file managers.
 *
 * Use this class in implementations of IProjectFileManager to simplify
 * the management of IProjectFilter instances for projects managed by
 * your file manager.
 *
 * NOTE: This interface is _not_ threadsafe. But you can use filtersForProject()
 * to implement thread safe filtering.
 *
 * @author Milian Wolff
 */
class KDEVPLATFORMPROJECT_EXPORT ProjectFilterManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectFilterManager(QObject* parent = 0);
    virtual ~ProjectFilterManager();

    /**
     * @copydoc IProjectFilter::isValid
     */
    bool isValid(const Path& path, bool isFolder, IProject* project) const;

    /**
     * Manage the filters of the given project.
     */
    void add(IProject* project);

    /**
     * Remove the managed filters of the given project.
     */
    void remove(IProject* project);

    /**
     * TODO: remove this once the cmake manager got cleaned up to not need this
     *       anymore.
     *
     * @return true if the project is managed, false otherwise.
     */
    bool isManaged(IProject* project) const;

    /**
     * @return the current list of project filters for a given managed project.
     *
     * This can be used to implement a thread safe project filtering
     */
    QVector<QSharedPointer<IProjectFilter> > filtersForProject(IProject* project) const;

private:
    struct Private;
    QScopedPointer<Private> d;

    Q_PRIVATE_SLOT(d, void pluginLoaded( KDevelop::IPlugin* ) )
    Q_PRIVATE_SLOT(d, void unloadingPlugin( KDevelop::IPlugin* ) )
    Q_PRIVATE_SLOT(d, void filterChanged( KDevelop::IProjectFilterProvider*, KDevelop::IProject* ) )
};

}

#endif // KDEVPLATFORM_PROJECTFILTERMANAGER_H
