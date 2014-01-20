/* This file is part of the KDE project
   Copyright 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2002 Simon Hausmann <hausmann@kde.org>
   Copyright 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright     2006 Matt Rogers <mattr@kde.org>
   Copyright     2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef KDEVPLATFORM_DUMBPROJECT_H
#define KDEVPLATFORM_DUMBPROJECT_H

#include <shell/project.h>

#include <KUrl>

template<typename T> class QList;

class KJob;

namespace KDevelop
{

class IProjectFileManager;
class IBuildSystemManager;
class ProjectFileItem;
class PersistentHash;

class DumbProject : public Project
{
    Q_OBJECT
public:
    DumbProject(QObject *parent = 0);

    /// Destructor.
    virtual ~DumbProject();

    /** Get a list of all files in the project */
    virtual int fileCount() const;
    virtual ProjectFileItem* fileAt( int i ) const;

    virtual QList<ProjectFileItem*> files() const;

    virtual QList<ProjectFileItem*> filesForUrl( const KUrl& ) const;

    QString projectTempFile() const;
    QString developerTempFile() const;
    KUrl developerFileUrl() const;
    virtual void reloadModel();
    virtual KUrl projectFileUrl() const;
    virtual KSharedConfig::Ptr projectConfiguration() const;

public Q_SLOTS:
    virtual bool open(const KUrl &projectFileUrl);

    virtual void close();

    /**
     * @brief Get the project folder
     * @return The canonical absolute directory of the project.
     */
    virtual Q_SCRIPTABLE const KUrl folder() const;

    /** Returns the name of the project. */
    virtual Q_SCRIPTABLE QString name() const;

    /**
     * Get the file manager for the project
     *
     * @return the file manager for the project, if one exists; otherwise null
     */
    IProjectFileManager* projectFileManager() const;

    /**
     * Get the build system manager for the project
     *
     * @return the build system manager for the project, if one exists; otherwise null
     */
    IBuildSystemManager* buildSystemManager() const;

    IPlugin* versionControlPlugin() const;

    /**
     * Get the plugin that manages the project
     * This can be used to get other interfaces like IBuildSystemManager
     */
    IPlugin* managerPlugin() const;

    /**
     * Set the manager plugin for the project.
     */
    void setManagerPlugin( IPlugin* manager );

    /**
     * With this the top-level project item can be retrieved
     */
    ProjectFolderItem* projectItem() const;

    /**
     * Find the url relative to the project directory equivalent to @a absoluteUrl.
     * This function does not check to see if the file is contained within the
     * project; for that, use inProject().
     *
     * @param absoluteUrl Absolute url to convert
     * @deprecated use KUrl::relativeUrl instead
     * @returns absoluteUrl relative to projectDirectory()
     **/
    KUrl relativeUrl(const KUrl& absoluteUrl) const;

private:
    class DumbProjectPrivate* const d;
};

}
#endif
