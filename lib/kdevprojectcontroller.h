/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#ifndef KDEVPROJECTCONTROLLER_H
#define KDEVPROJECTCONTROLLER_H

#include <QObject>

#include "kdevexport.h"

#include <kurl.h>

class KDevProject;

class KDEVINTERFACES_EXPORT KDevProjectController : public QObject
{
    friend class KDevCore;
    Q_OBJECT
public:
    KDevProjectController( QObject *parent = 0 );
    virtual ~KDevProjectController();

    /** Release all resources that depend on other KDevCore objects */
    void cleanUp();

    QString name() const;
    void setName( const QString &name );

    KUrl localFile() const;
    void setLocalFile( const KUrl &localFile );

    KUrl globalFile() const;
    void setGlobalFile( const KUrl &globalFile );

    KUrl projectsDirectory() const;
    void setProjectsDirectory( const KUrl &projectsDir );

    bool isLoaded() const;
    KUrl projectDirectory() const;

    KDevProject* activeProject() const;

public Q_SLOTS:
    bool openProject( const KUrl &KDev4ProjectFile = KUrl() );
    bool closeProject();

Q_SIGNALS:
    void projectOpened();
    void projectClosed();

private:
    void init();

    //FIXME Do not load all of this just for the project being opened...
    void legacyLoading();
    bool loadProjectPart( const QString &projectManager );
    void unloadProjectPart();

private:
    QString m_name;
    KUrl m_localFile;
    KUrl m_globalFile;
    KUrl m_projectsDir;
    bool m_isLoaded;
    KDevProject* m_project;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
