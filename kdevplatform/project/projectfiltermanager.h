/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
class ProjectFilterManagerPrivate;

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
    explicit ProjectFilterManager(QObject* parent = nullptr);
    ~ProjectFilterManager() override;

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
    const QScopedPointer<class ProjectFilterManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectFilterManager)

    Q_PRIVATE_SLOT(d_func(), void filterChanged( KDevelop::IProjectFilterProvider*, KDevelop::IProject* ) )
};

}

#endif // KDEVPLATFORM_PROJECTFILTERMANAGER_H
