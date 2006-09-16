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
#ifndef IMPORTPROJECTJOB_H
#define IMPORTPROJECTJOB_H

#include <kjob.h>
#include "kdevprojectmodel.h"
#include "kdevexport.h"

class KDevFileManager;

class KDEVPROJECTMANAGER_EXPORT ImportProjectJob: public KJob
{
    Q_OBJECT
public:
    ImportProjectJob(QStandardItem *folder, KDevFileManager *importer);
    virtual ~ImportProjectJob();

public:
    void start();

protected:
    void startNextJob(KDevProjectFolderItem *folder);
    void slotResult(KJob *job);
    void processList();

private:
    KDevProjectFolderItem *m_folder;
    KDevFileManager *m_importer;
    QList<KDevProjectFolderItem*> m_workingList;
};

#endif // IMPORTPROJECTJOB_H

// kate: space-indent on; indent-width 2; replace-tabs on;
