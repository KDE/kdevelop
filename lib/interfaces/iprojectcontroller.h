/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Anreas Pakulat <apaku@gmx.de>

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

#ifndef IPROJECTCONTROLLER_H
#define IPROJECTCONTROLLER_H

#include <QObject>
#include <kurl.h>
#include "kdevexport.h"

namespace Koncrete
{

class IPlugin;
class IProject;

class KDEVPLATFORM_EXPORT IProjectController : public QObject
{
    Q_OBJECT
public:
    IProjectController( QObject *parent = 0 );
    virtual ~IProjectController();

    virtual KUrl localFile() const = 0;
    virtual void setLocalFile( const KUrl &localFile ) = 0;

    virtual KUrl globalFile() const = 0;
    virtual void setGlobalFile( const KUrl &globalFile ) = 0;

    virtual KUrl projectsDirectory() const = 0;
    virtual void setProjectsDirectory( const KUrl &projectsDir ) = 0;

    virtual bool isLoaded() const = 0;
    virtual KUrl projectDirectory() const = 0;

    virtual IProject* activeProject() const = 0;

public Q_SLOTS:
    virtual bool openProject( const KUrl &KDev4ProjectFile = KUrl() ) = 0;
    virtual bool closeProject() = 0;

Q_SIGNALS:
    void projectOpened();
    void projectClosing();
    void projectClosed();
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
