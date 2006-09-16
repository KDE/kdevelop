/*  This file is part of KDevelop
    Copyright (C) 2006 Matt Rogers <mattr@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVBUILDABLEMANAGER_H
#define KDEVBUILDABLEMANAGER_H

#include <kdevfilemanager.h>

class KDevProjectBuilder;
typedef QList<KDevProjectFileItem*> FileItemList;

/**
 * Manage the file and build system of the project.
 * @author Matt Rogers <mattr@kde.org>, Hamish Rodda <rodda@kde.org>
 */
class KDEVINTERFACES_EXPORT KDevBuildManager : public KDevFileManager
{
    Q_OBJECT

public:
    KDevBuildManager(KInstance*, QObject* parent);
    ~KDevBuildManager();

    enum BuildFeature
    {
        Includes, ///< This project supports passing include directives to
                  ///< the compiler
        Defines   ///< This project supports passing preprocessor defines to
                  ///< compiler
    };
    Q_DECLARE_FLAGS( BuildFeatures, BuildFeature )

    /**
     * Provide access to the builder
     */
    virtual KDevProjectBuilder* builder() const = 0;

    /**
     * Provide a list of include directories.
     */
    virtual KUrl::List includeDirectories() const;

    /**
     * Provide a list of files that contain the preprocessor defines for the
     * project
     */
    virtual KUrl::List preprocessorDefines() const;

    /** @return The Makefile associated with the specified @p folder.
     *   @note The Makefile list must contain absolute file names
     *
     * For instance, in <b>Automake</b> projects you can use something like:
     * @code
     * return dom->name() + "/Makefile.am";
     * @endcode
     */
    virtual KUrl findMakefile(KDevProjectFolderItem* folder) const;

    /** @return The list of the Makefiles from the @p folder.
    @note The Makefile list must contains absolute file names */
    virtual KUrl::List findMakefiles(KDevProjectFolderItem* folder) const;

    /**
     * Create a new target
     *
     * Creates the target specified by @p target to the folder @p parent and
     * modifies the underlying build system if needed
     */
    virtual KDevProjectTargetItem* createTarget(const QString& target, KDevProjectFolderItem *parent) = 0;

    /**
     * Add a file to a target
     *
     * Adds the file specified by @pfile to the target @p parent and modifies
     * the underlying build system if needed.
     */
    virtual bool addFileToTarget(KDevProjectFileItem *file, KDevProjectTargetItem *parent) = 0;

    /**
     * Remove a target
     *
     * Removes the target specified by @p target and
     * modifies the underlying build system if needed.
     */
    virtual bool removeTarget(KDevProjectTargetItem *target) = 0;

    /**
     * Remove a file from a target
     *
     * Removes the file specified by @p file from the folder @parent and
     * modifies the underlying build system if needed. The file is not removed
     * from the folder it is in
     */
    virtual bool removeFileFromTarget(KDevProjectFileItem *file, KDevProjectTargetItem *parent) = 0;

    /**
     * Get the toplevel build directory for the project
     */
    virtual KUrl buildDirectory() const = 0;
    
    /**
     * Get a list of all the targets in this project
     *
     * The list returned by this function should be checked to verify it is not
     * empty before using it
     *
     * @return The list of targets for this project
     * @todo implement
     */
    //QList<KDevProjectTargetItem*> targets() const;


};

Q_DECLARE_OPERATORS_FOR_FLAGS( KDevBuildManager::BuildFeatures )

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
