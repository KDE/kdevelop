/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IBUILDSYSTEMMANAGER_H
#define KDEVPLATFORM_IBUILDSYSTEMMANAGER_H

#include "iprojectfilemanager.h"

#include <project/projectexport.h>
#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

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

    ~IBuildSystemManager() override;

    enum BuildFeature
    {
        Includes /**< This project supports passing include directives to the compiler */,
        Defines  /**< This project supports passing preprocessor defines to compiler */
    };
    Q_DECLARE_FLAGS( BuildFeatures, BuildFeature )

    /**
     * Provide access to the builder. This method never returns
     * null, if it does that is a bug in the plugin. A BuildSystemManager
     * always has a project builder associated with it.
     */
    virtual IProjectBuilder* builder() const = 0;

    /**
     * Provide a list of include directories.
     */
    virtual Path::List includeDirectories(ProjectBaseItem*) const = 0;

    /**
     * Provide a list of framework directories.
     */
    virtual Path::List frameworkDirectories(ProjectBaseItem*) const = 0;

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
     * Returns if the build system has information specific to @p item
     */
    virtual bool hasBuildInfo(ProjectBaseItem* item) const = 0;

    /**
     * Get the toplevel build directory for the project
     */
    virtual Path buildDirectory(ProjectBaseItem*) const = 0;

    /**
     * @returns the extra arguments that will be passed to the compiler when building @p item
     */
    virtual QString extraArguments(ProjectBaseItem* item) const = 0;

    /**
     * @returns the absolute path to the tool that will be used or an empty path if unknown
     */
    virtual Path compiler(KDevelop::ProjectTargetItem* p) const = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IBuildSystemManager::BuildFeatures)
}

Q_DECLARE_INTERFACE( KDevelop::IBuildSystemManager, "org.kdevelop.IBuildSystemManager" )

#endif
