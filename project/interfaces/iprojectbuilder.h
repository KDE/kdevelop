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
#ifndef IPROJECTBUILDER_H
#define IPROJECTBUILDER_H

#include <iextension.h>
#include <projectexport.h>

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
    virtual bool install(ProjectBaseItem* item) = 0;

    /**
     * Builds the given project item, exact behaviour depends
     * on the implementation
     */
    virtual bool build(ProjectBaseItem *dom) = 0;

    /**
     * Cleans the given project item, exact behaviour depends
     * on the implementation. The cleaning should only include
     * output files, like C/C++ object files, binaries, files
     * that the builder needs shouldn't be removed.
     */
    virtual bool clean(ProjectBaseItem *dom) = 0;

    /**
     * Configures the given project, exact behaviour depends
     * on the implementation. After calling this a build() call should
     * succeed (given the code doesn't have errors).
     *
     * This function is optional, the default implementation does nothing.
     */
    virtual bool configure(IProject*);

    /**
     * Prunes the given project, exact behaviour depends
     * on the implementation. Additionally to what clean() does this may
     * also remove files used for the builder (or a "subbuilder"), for example
     * generated Makefiles in QMake/CMake/Automake projects
     *
     * This function is optional, the default implementation does nothing.
     */
    virtual bool prune(IProject*);

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

KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IProjectBuilder, "org.kdevelop.IProjectBuilder" )
Q_DECLARE_INTERFACE( KDevelop::IProjectBuilder, "org.kdevelop.IProjectBuilder" )

#endif
