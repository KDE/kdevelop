/* This file is part of the KDevelop platform
   Copyright (C) 2006 Matt Rogers <mattr@kde.org>

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

#ifndef KDEVBUILDABLEPROJECT_H
#define KDEVBUILDABLEPROJECT_H

#include "kdevproject.h"

class KDevBuildManager;
class KDevProjectTargetItem;

/**
 * This class represents a project that is buildable, in other words it
 * requires going through some sort of compile process before something that
 * can be executed is generated.
 */
class KDevBuildableProject : public KDevProject
{
Q_OBJECT
public:
    KDevBuildableProject(KInstance *instance, QObject *parent = 0);
    virtual ~KDevBuildableProject();

    /**
     * Get the buildable file manager for the project.
     *
     * @return the file manager for the project, if one exists; otherwise null
     */
    KDevBuildManager* buildManager() const;

    /**
     * Set the file manager for the project.
     */
    void setBuildManager( KDevBuildManager* fileManager ) const;

    /**
     * Get the canonical build directory of the project.
     *
     * If a seperate build directory is not supported, this should return
     * the same url as projectDirectory(). The url will not point to a symbolic
     * link or have redundant "." or ".." elements
     *
     * Remote URLs for a project's build directory are not supported and should
     * not be used.
     * 
     * @return The canonical build directory of the project.
     */
    virtual KUrl buildDirectory() const = 0;

    /**
     * Get the environment to use when running the program
     *
     * @return the environment variables that should be set before running
     * mainProgram()
     */
    virtual DomUtil::PairList runEnvironmentVars() const = 0;

    /**
     * Absolute path (directory) from where the mainProgram() should be run.
     **/
    virtual KUrl runDirectory() const = 0;

    /**The command line arguments that the mainProgram() should be run with.*/
    virtual QString runArguments() const = 0;

signals:
    /**
     * Emitted when one compile related command (make, make install, make ...)
     * ends sucessfuly. Used to reparse the files after a sucessful
     * compilation.
     */
    void projectCompiled();
};

#endif
//kate: indent-mode cstyle; indent-width 4; space-indent on; auto-insert-doxygen on;
