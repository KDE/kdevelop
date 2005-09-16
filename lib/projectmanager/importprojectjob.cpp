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

#include "importprojectjob.h"

#include <kdevprojectimporter.h>

#include <kglobal.h>
#include <kapplication.h>
#include <kdebug.h>

ImportProjectJob::ImportProjectJob(KDevProjectFolderItem *folder, KDevProjectImporter *importer)
    : KIO::Job(false),
      m_folder(folder),
      m_importer(importer)
{
}

ImportProjectJob::~ImportProjectJob()
{
}

void ImportProjectJob::start()
{
    startNextJob(m_folder);
}

ImportProjectJob *ImportProjectJob::importProjectJob(KDevProjectFolderItem *folder, KDevProjectImporter *importer)
{
    return new ImportProjectJob(folder, importer);
}

void ImportProjectJob::startNextJob(KDevProjectFolderItem *dom)
{
    m_workingList += m_importer->parse(dom);
    processList();
}

void ImportProjectJob::processList()
{
    if (!m_workingList.isEmpty()) {
        KDevProjectFolderItem *folder = m_workingList.first();
        m_workingList.pop_front();

        startNextJob(folder);
    } else
        emitResult();
}

void ImportProjectJob::slotResult(KIO::Job *job)
{
    if (m_workingList.isEmpty())
        KIO::Job::slotResult(job);
    else
        processList();
}

// kate: space-indent on; indent-width 2; replace-tabs on;

