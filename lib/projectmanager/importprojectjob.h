/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef IMPORTPROJECTJOB_H
#define IMPORTPROJECTJOB_H

#include <kio/job.h>
#include <kdevprojectmodel.h>

class KDevProjectImporter;

class ImportProjectJob: public KIO::Job
{
    Q_OBJECT
protected:
    ImportProjectJob(ProjectFolderDom folder, KDevProjectImporter *importer);
    virtual ~ImportProjectJob();

    
public:
    static ImportProjectJob *importProjectJob(ProjectFolderDom folder, KDevProjectImporter *importer);
    void start();
    
protected:
    void startNextJob(ProjectFolderDom folder);
    void slotResult(KIO::Job *job);
    void processList();
    
private:
    ProjectFolderDom m_folder;
    KDevProjectImporter *m_importer;
    ProjectFolderList m_workingList;
};

#endif // IMPORTPROJECTJOB_H
