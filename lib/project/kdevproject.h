/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright     2006 Matt Rogers <mattr@kde.org>

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
#ifndef KDEVPROJECT_H
#define KDEVPROJECT_H

#include <QObject>

#include <kurl.h>

#include "kdevexport.h"

template<typename T> class QList;

class KJob;
class KInstance;
class KDevFileManager;
class KDevProjectModel;
class KDevProjectItem;
class KDevProjectFileItem;
class KDevProjectFolderItem;
class KDevPersistentHash;

/**
 * \brief Object which represents a KDevelop project
 *
 * Provide better descriptions
 */
class KDEVINTERFACES_EXPORT KDevProject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kdevelop.Project")
public:
    /**
     * Constructs a project.
     *
     * @param parent The parent object for the plugin.
     */
    KDevProject(QObject *parent = 0);

    /// Destructor.
    virtual ~KDevProject();

    /** Project model accessor */
    KDevProjectModel* model() const;


public Q_SLOTS:
    /**
     * @brief Open a project
     * This method opens a project and starts the process of loading the
     * data for the project from disk.
     * @param projectFileUrl The url pointing to the location of the project
     * file to load
     * The project name is taken from the Name key in the project file in
     * the 'General' group
     */
    virtual Q_SCRIPTABLE void open(const KUrl &projectFileUrl);

    /** This method is invoked when the project needs to be closed. */
    virtual Q_SCRIPTABLE void close();

    /**
     * @brief Get the project folder
     * @return The canonical absolute directory of the project. 
     */
    virtual Q_SCRIPTABLE KUrl folder() const;

    /** Returns the name of the project. */
    virtual Q_SCRIPTABLE QString name() const;

    /** Get a list of all files in the project */
    virtual QList<KDevProjectFileItem*> allFiles();

    /**
     * Get the file manager for the project
     *
     * @return the file manager for the project, if one exists; otherwise null
     */
    KDevFileManager* fileManager() const;

    /**
     * Set the file manager for the project.
     */
    void setFileManager( KDevFileManager* fileManager );

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

    /**
     * Returns the absolute url corresponding to the given \a relativeUrl and
     * the project directory.
     *
     * @param relativeUrl Relative url to convert
     *
     * @returns the absolute URL relative to projectDirectory()
     **/
    KUrl urlRelativeToProject(const KUrl& relativeUrl) const;

    /**
     * Check if the url specified by @a url is part of the project.
     * @a url can be either a relative url (to the project directory) or
     * an absolute url.
     *
     * @param url the url to check
     *
     * @return true if the url @a url is a part of the project.
     */
    bool inProject(const KUrl &url) const;

    /**
     * The persistent AST storage for this project.
     */
    KDevPersistentHash *persistentHash() const;

private Q_SLOTS:
    void importDone(KJob*);

private:
    QList<KDevProjectFileItem*> recurseFiles( KDevProjectItem* projectItem );

private:
    class Private;
    Private* const d;
};

#endif
//kate: indent-mode cstyle; auto-insert-doxygen on; indent-width 4; space-indent on;
