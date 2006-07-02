/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>

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

#include "kdevplugin.h"
#include "domutil.h"

#include <qstringlist.h>
#include <QMap>

#include <kurl.h>
/**
@file kdevproject.h
KDevelop project interface.
*/

class QTimer;
class KInstance;
class KDevFileManager;
class KDevProjectFileItem;
class KDevProjectFolderItem;

/**
 * \short Object which represents a KDevelop project
 *
 * KDevProject is a container for everything which relates to the files
 * within a development project.  It maintains a list of files contained
 * in it.
 *
 * Unless otherwise specified, all returned URLs are absolute urls.  Provided
 * URLs may be relative to the project directory or absolute.
 */
class KDEVINTERFACES_EXPORT KDevProject: public KDevPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kdevelop.Project")
public:
    /**
     * Constructs a project.
     *
     * @param info information about the plugin - plugin internal and generic
     * (GUI) name, description, a list of authors, etc. That information is used to show
     * plugin information in various places like "about application" dialog, plugin selector
     * dialog, etc. Plugin does not take ownership on info object, also its lifetime should
     * be equal to the lifetime of the plugin.
     * @param parent The parent object for the plugin.
     */
    KDevProject(KInstance *instance, QObject *parent = 0);

    /// Destructor.
    virtual ~KDevProject();


public Q_SLOTS:
    /**This method is invoked when the project is opened
    (i.e. actually just after this class has been
    instantiated).
    @param dirName The project directory, which should afterwards be returned by
    the projectDirectory() method.
    @param projectName The project name, which is equivalent
    to the project file name without the suffix.*/
    virtual Q_SCRIPTABLE void openProject(const KUrl &dirName, const QString &projectName) = 0;

    /**This method is invoked when the project is about to be closed.*/
    virtual Q_SCRIPTABLE void closeProject() = 0;

    /**@return The canonical absolute directory of the project. Canonical means that
    a path does not contain symbolic links or redundant "." or ".." elements.*/
    virtual Q_SCRIPTABLE KUrl projectDirectory() const = 0;

    /**Returns the name of the project.*/
    virtual Q_SCRIPTABLE QString projectName() const = 0;

    virtual QList<KDevProjectFileItem*> allFiles() = 0;

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
     *
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
    KUrl absoluteUrl(const KUrl& relativeUrl) const;

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

private:
    class KDevProjectPrivate* const d;
};

#endif
//kate: indent-mode cstyle; auto-insert-doxygen on; indent-width 4; space-indent on;
