/*  This file is part of KDevelop
    Copyright 2006 Matt Rogers <mattr@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_IBUILDSYSTEMMANAGER_H
#define KDEVPLATFORM_IBUILDSYSTEMMANAGER_H

#include <KUrl>

#include "iprojectfilemanager.h"

#include "../projectexport.h"

namespace KDevelop
{

class IProjectBuilder;
class ProjectTargetItem;
/**
 * Manages the build system of the project.
 *
 * Use/Implement the IProjectFileManager interface to manage files.
 *
 * @author Matt Rogers <mattr@kde.org>, Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMPROJECT_EXPORT IBuildSystemManager : public virtual IProjectFileManager
{
public:

    virtual ~IBuildSystemManager();

    enum BuildFeature
    {
        Includes /**< This project supports passing include directives to the compiler */,
        Defines  /**< This project supports passing preprocessor defines to compiler */
    };
    Q_DECLARE_FLAGS( BuildFeatures, BuildFeature )

    /**
     * Provide access to the builder. This method never returns
     * null, if it does thats a bug in the plugin. A BuildSystemManager
     * always has a project builder associated with it.
     */
    virtual IProjectBuilder* builder() const = 0;

    /**
     * Provide a list of include directories.
     */
    virtual KUrl::List includeDirectories(ProjectBaseItem*) const = 0;

    /**
     * Provide a list of preprocessor defines for the project item
     */
    virtual QHash<QString,QString> defines(ProjectBaseItem*) const = 0;

    /**
     * Create a new target
     *
     * Creates the target specified by @p target to the folder @p parent and
     * modifies the underlying build system if needed
     */
    virtual ProjectTargetItem* createTarget(const QString& target, ProjectFolderItem *parent) = 0;

    /**
     * Remove a target
     *
     * Removes the target specified by @p target and
     * modifies the underlying build system if needed.
     */
    virtual bool removeTarget(ProjectTargetItem *target) = 0;

    /**
     * Get a list of all the targets in this project
     *
     * The list returned by this function should be checked to verify it is not
     * empty before using it
     *
     * @return The list of targets for this project
     * @todo implement
     */
    virtual QList<ProjectTargetItem*> targets(ProjectFolderItem*) const = 0;

    /**
     * Add a file to a target
     *
     * Adds the file specified by @p file to the target @p parent and modifies
     * the underlying build system if needed.
     */
    virtual bool addFilesToTarget(const QList<ProjectFileItem*> &files, ProjectTargetItem *target) = 0;

    /**
     * Remove files from targets
     *
     * Removes the files from the targets they are paired with (@p targetFiles)
     * Files are not removed from the folders or the filesystem.
     */
    virtual bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem*> &files) = 0;

    /**
     * Get the toplevel build directory for the project
     */
    virtual KUrl buildDirectory(ProjectBaseItem*) const = 0;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::IBuildSystemManager::BuildFeatures )

Q_DECLARE_INTERFACE( KDevelop::IBuildSystemManager, "org.kdevelop.IBuildSystemManager" )

#endif


