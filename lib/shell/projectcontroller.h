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

#ifndef KDEVPROJECTCONTROLLER_H
#define KDEVPROJECTCONTROLLER_H

#include "iprojectcontroller.h"

#include "kdevexport.h"

class KRecentFilesAction;

namespace Koncrete
{

class IProject;
class Core;

class KDEVPLATFORM_EXPORT ProjectController : public IProjectController
{
    Q_OBJECT
    friend class Core;
public:
    ProjectController( Core* core );
    virtual ~ProjectController();

    virtual IProject* projectAt( int ) const;
    virtual int projectCount() const;

//     KUrl localFile() const;
//     void setLocalFile( const KUrl &localFile );
//
//     KUrl globalFile() const;
//     void setGlobalFile( const KUrl &globalFile );
//
//     KUrl projectsDirectory() const;
//     void setProjectsDirectory( const KUrl &projectsDir );
//
//     bool isLoaded() const;
//     KUrl projectDirectory() const;
//
//     IProject* activeProject() const;

    virtual ProjectModel* projectModel();

public Q_SLOTS:
    bool openProject( const KUrl &KDev4ProjectFile = KUrl() );
    bool closeProject( IProject* );

Q_SIGNALS:
    void projectOpened( IProject* );
    void projectClosing( IProject* );
    void projectClosed( IProject* );

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );


private:
    //FIXME Do not load all of this just for the project being opened...
    //void legacyLoading();
    void setupActions();
    void cleanup();
    bool loadProjectPart();

private:
    struct ProjectControllerPrivate* const d;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
