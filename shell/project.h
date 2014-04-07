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
#ifndef KDEVPLATFORM_SHELLPROJECT_H
#define KDEVPLATFORM_SHELLPROJECT_H

#include <kurl.h>

#include <interfaces/iproject.h>
#include <interfaces/istatus.h>

#include "shellexport.h"

template<typename T> class QList;

class KJob;

namespace KDevelop
{

class IProjectFileManager;
class IBuildSystemManager;
class ProjectFileItem;
class PersistentHash;

/**
 * \brief Object which represents a KDevelop project
 *
 * Provide better descriptions
 */
class KDEVPLATFORMSHELL_EXPORT Project : public IProject
{
    Q_OBJECT
public:
    /**
     * Constructs a project.
     *
     * @param parent The parent object for the plugin.
     */
    Project(QObject *parent = 0);

    /// Destructor.
    virtual ~Project();

    /** Get a list of all files in the project */
    virtual int fileCount() const;
    virtual ProjectFileItem* fileAt( int i ) const;

    virtual QList<ProjectFileItem*> files() const;

    virtual QList< ProjectBaseItem* > itemsForPath(const IndexedString& path) const;
    virtual QList< ProjectFileItem* > filesForPath(const IndexedString& file) const;
    virtual QList< ProjectFolderItem* > foldersForPath(const IndexedString& folder) const;

    KDE_DEPRECATED virtual QList< ProjectBaseItem* > itemsForUrl(const KUrl& url) const;
    KDE_DEPRECATED virtual QList<ProjectFileItem*> filesForUrl( const KUrl& ) const;
    KDE_DEPRECATED virtual QList<ProjectFolderItem*> foldersForUrl(const KUrl& ) const;

    QString projectTempFile() const;
    QString developerTempFile() const;
    KDE_DEPRECATED KUrl developerFileUrl() const;
    Path developerFile() const;
    virtual void reloadModel();
    virtual KDE_DEPRECATED KUrl projectFileUrl() const;
    virtual Path projectFile() const;
    virtual KSharedConfig::Ptr projectConfiguration() const;

    virtual void addToFileSet( ProjectFileItem* file );
    virtual void removeFromFileSet( ProjectFileItem* file );
    virtual QSet<IndexedString> fileSet() const;

    virtual bool isReady() const;

    /**
     * @brief Get the project folder
     * @return The canonical absolute directory of the project.
     */
    virtual KDE_DEPRECATED const KUrl folder() const;

    virtual Path path() const;

    /** Returns the name of the project. */
    virtual Q_SCRIPTABLE QString name() const;

public Q_SLOTS:
    /**
     * @brief Open a project
     *
     * This method opens a project and starts the process of loading the
     * data for the project from disk.
     *
     * @param projectFile The path pointing to the location of the project
     *                    file to load
     *
     * The project name is taken from the Name key in the project file in
     * the 'General' group
     */
    bool open(const Path &projectFile);

    /** This method is invoked when the project needs to be closed. */
    void close();

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
     * Check if the url specified by @a url is part of the project.
     * @a url can be either a relative url (to the project directory) or
     * an absolute url.
     *
     * @param url the url to check
     *
     * @return true if the url @a url is a part of the project.
     */
    bool inProject(const IndexedString &url) const;

    virtual void setReloadJob(KJob* job);

signals:
    /**
     * Internal signal to make IProjectController::projectAboutToOpen useful.
     */
    void aboutToOpen(KDevelop::IProject*);

private:
    Q_PRIVATE_SLOT(d, void importDone(KJob*))
    Q_PRIVATE_SLOT(d, void reloadDone(KJob*))

    class ProjectPrivate* const d;
};

} // namespace KDevelop
#endif
