/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPROJECTBUILDER_H
#define KDEVPLATFORM_IPROJECTBUILDER_H

#include <project/projectexport.h>

#include <QObject>
#include <QUrl>

class KJob;

namespace KDevelop
{
class IProject;
class ProjectBaseItem;

/**
@author Roberto Raggi

@short IProjectBuilder Base class for the Project Builders

Provides an interface that will be useful to perform tasks upon projects,
inspired on the unix-style way to build projects:

configure -> make -> make install

It will provide a job for each of the tasks and a signal to make sure it
completed successfully.
*/
class KDEVPLATFORMPROJECT_EXPORT IProjectBuilder
{
public:

    virtual ~IProjectBuilder();

    /**
     * Installs the given project @p item, exact behaviour depends
     * on the implementation.
     *
     * @param specificPrefix defines where the project will be installed.
     */
    virtual KJob* install(ProjectBaseItem* item, const QUrl &specificPrefix = {}) = 0;

    /**
     * Builds the given project @p item, exact behaviour depends
     * on the implementation
     */
    virtual KJob* build(ProjectBaseItem *item) = 0;

    /**
     * Cleans the given project @p item, exact behaviour depends
     * on the implementation. The cleaning should only include
     * output files, like C/C++ object files, binaries, files
     * that the builder needs shouldn't be removed.
     */
    virtual KJob* clean(ProjectBaseItem *item) = 0;

    /**
     * Configures the given @p project, exact behaviour depends
     * on the implementation. After calling this a build() call should
     * succeed (given the code doesn't have errors).
     *
     * This function is optional, the default implementation does nothing.
     */
    virtual KJob* configure(IProject* project);

    /**
     * Prunes the given @p project, exact behaviour depends
     * on the implementation. Additionally to what clean() does this may
     * also remove files used for the builder (or a "subbuilder"), for example
     * generated Makefiles in QMake/CMake/Automake projects
     *
     * This function is optional, the default implementation does nothing.
     */
    virtual KJob* prune(IProject* project);

    /**
     * Provide access to the builders related to the @p project.
     * The list returned by this method is used to select the appropriate config pages for a project.
     * This method may safely return an empty list, as does the default implementation.
     */
    virtual QList<IProjectBuilder*> additionalBuilderPlugins( IProject* project ) const;

Q_SIGNALS:
    /**
     * Emitted when the build for the given item was finished
     */
    void built(ProjectBaseItem *dom);
    /**
     * Emitted when the install for the given item was finished
     */
    void installed(ProjectBaseItem*);
    /**
     * Emitted when the clean for the given item was finished
     */
    void cleaned(ProjectBaseItem*);
    /**
     * Emitted when any of the scheduled actions for the given item was failed
     */
    void failed(ProjectBaseItem *dom);
    /**
     * Emitted when the configure for the given item was finished
     */
    void configured(IProject*);
    /**
     * Emitted when the pruning for the given item was finished
     */
    void pruned(IProject*);
};

}

Q_DECLARE_INTERFACE( KDevelop::IProjectBuilder, "org.kdevelop.IProjectBuilder" )

#endif
