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

#include <QtCore/QObject>
#include <QtCore/QList>
#include <kurl.h>
#include "interfacesexport.h"


namespace KDevelop
{

class IProject;
class ProjectModel;
class ProjectBaseItem;

class KDEVPLATFORMINTERFACES_EXPORT IProjectController : public QObject
{
    Q_OBJECT
public:
    IProjectController( QObject *parent = 0 );
    virtual ~IProjectController();

    virtual IProject* projectAt( int ) const = 0;
    virtual int projectCount() const = 0;
    virtual QList<IProject*> projects() const = 0;

    virtual ProjectModel* projectModel() = 0;
//     virtual IProject* currentProject() const = 0;

    virtual IProject* findProjectForUrl( const KUrl& ) const = 0;

public Q_SLOTS:
    virtual bool openProject( const KUrl &KDev4ProjectFile = KUrl() ) = 0;
    virtual bool closeProject( IProject* ) = 0;
//     virtual void changeCurrentProject( KDevelop::ProjectBaseItem* ) = 0;

Q_SIGNALS:
    void projectOpened( KDevelop::IProject* );
    void projectClosing( KDevelop::IProject* );
    void projectClosed( KDevelop::IProject* );
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
