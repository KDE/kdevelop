/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef KDEVPLATFORM_IPROJECTBUILDER_H
#define KDEVPLATFORM_IPROJECTBUILDER_H

#include "projectexport.h"
#include <QtCore/QObject>

class KJob;

namespace KDevelop
{
class IProject;
class ProjectBaseItem;

/**
@author Roberto Raggi

@short IProjectBuilder Base class for the Project Builders

Describes a <b>Project Builder</b> to KDevelop's Project Manager.
*/
class KDEVPLATFORMPROJECT_EXPORT IProjectBuilder
{
public:

    virtual ~IProjectBuilder();

    /**
     * Installs the given project item, exact behaviour depends
     * on the implementation
     */
    virtual KJob* install(ProjectBaseItem* item) = 0;

    /**
     * Builds the given project item, exact behaviour depends
     * on the implementation
     */
    virtual KJob* build(ProjectBaseItem *dom) = 0;

    /**
     * Cleans the given project item, exact behaviour depends
     * on the implementation. The cleaning should only include
     * output files, like C/C++ object files, binaries, files
     * that the builder needs shouldn't be removed.
     */
    virtual KJob* clean(ProjectBaseItem *dom) = 0;

    /**
     * Configures the given project, exact behaviour depends
     * on the implementation. After calling this a build() call should
     * succeed (given the code doesn't have errors).
     *
     * This function is optional, the default implementation does nothing.
     */
    virtual KJob* configure(IProject*);

    /**
     * Prunes the given project, exact behaviour depends
     * on the implementation. Additionally to what clean() does this may
     * also remove files used for the builder (or a "subbuilder"), for example
     * generated Makefiles in QMake/CMake/Automake projects
     *
     * This function is optional, the default implementation does nothing.
     */
    virtual KJob* prune(IProject*);

    /**
     * Provide access to the builders related to this one.
     * The list returned by this method is used to select the appropriate KCMs for a project.
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
