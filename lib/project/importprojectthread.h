/* This file is part of KDevelop
    Copyright (C) 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#ifndef IMPORTPROJECTTHREAD_H
#define IMPORTPROJECTTHREAD_H

#include <threadweaver/Job.h>

namespace KDevelop
{

class ProjectFolderItem;
class IProjectFileManager;

class ImportProjectThread : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    ImportProjectThread( QObject *parent );
    virtual ~ImportProjectThread();

    void setProjectFolderItem( ProjectFolderItem *folder );
    void setProjectFileManager( IProjectFileManager *importer );

protected:
    void run();

private:
    struct ImportProjectThreadPrivate* const d;
};

}

#endif
