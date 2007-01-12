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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "importprojectjob.h"

#include "kdevfilemanager.h"

#include <kglobal.h>
#include <kdebug.h>

ImportProjectJob::ImportProjectJob(QStandardItem *folder, KDevFileManager *importer)
    : KJob(0),
      m_importer(importer)
{
    if ( folder->type() == KDevProjectItem::Folder || 
         folder->type() == KDevProjectItem::BuildFolder )
    {
        m_folder = dynamic_cast<KDevProjectFolderItem*>( folder );
    }
}

ImportProjectJob::~ImportProjectJob()
{
}

void ImportProjectJob::start()
{
    if ( m_folder )
        startNextJob(m_folder);

    emitResult();
}

void ImportProjectJob::startNextJob(KDevProjectFolderItem *dom)
{
  m_workingList += m_importer->parse(dom);
    while ( !m_workingList.isEmpty() )
    {
      KDevProjectFolderItem *folder = m_workingList.first();
      m_workingList.pop_front();
      
      startNextJob(folder);
    }
}

void ImportProjectJob::processList()
{
}

#include "importprojectjob.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;

