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
class ProjectPrivate;

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
    explicit Project(QObject *parent = nullptr);
    ~Project() override;

    QList< ProjectBaseItem* > itemsForPath(const IndexedString& path) const override;
    QList< ProjectFileItem* > filesForPath(const IndexedString& file) const override;
    QList< ProjectFolderItem* > foldersForPath(const IndexedString& folder) const override;

    QString projectTempFile() const;
    QString developerTempFile() const;
    Path developerFile() const;
    void reloadModel() override;
    Path projectFile() const override;
    KSharedConfigPtr projectConfiguration() const override;

    void addToFileSet( ProjectFileItem* file ) override;
    void removeFromFileSet( ProjectFileItem* file ) override;
    QSet<IndexedString> fileSet() const override;

    bool isReady() const override;

    Path path() const override;

    Q_SCRIPTABLE QString name() const override;

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

    void close() override;

    IProjectFileManager* projectFileManager() const override;
    IBuildSystemManager* buildSystemManager() const override;
    IPlugin* versionControlPlugin() const override;

    IPlugin* managerPlugin() const override;

    /**
     * Set the manager plugin for the project.
     */
    void setManagerPlugin( IPlugin* manager );

    ProjectFolderItem* projectItem() const override;

    /**
     * Check if the url specified by @a url is part of the project.
     * @a url can be either a relative url (to the project directory) or
     * an absolute url.
     *
     * @param url the url to check
     *
     * @return true if the url @a url is a part of the project.
     */
    bool inProject(const IndexedString &url) const override;

    void setReloadJob(KJob* job) override;

Q_SIGNALS:
    /**
     * Internal signal to make IProjectController::projectAboutToOpen useful.
     */
    void aboutToOpen(KDevelop::IProject*);

private:
    const QScopedPointer<class ProjectPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Project)
};

} // namespace KDevelop
#endif
