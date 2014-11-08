/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#include "projectmodel.h"

#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>


#include <interfaces/iprojectfilemanager.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <QPointer>
#include <KLocalizedString>

namespace KDevelop
{

class ImportProjectJobPrivate
{
public:
    ImportProjectJobPrivate() : cancel(false) {}
    ProjectFolderItem *m_folder;
    IProjectFileManager *m_importer;
    QFutureWatcher<void> *m_watcher;
    QPointer<IProject> m_project;
    bool cancel;

    void import(ProjectFolderItem* folder)
    {
        foreach(ProjectFolderItem* sub, m_importer->parse(folder)) {
            if(!cancel) {
                import(sub);
            }
        }
    }
};

ImportProjectJob::ImportProjectJob(ProjectFolderItem *folder, IProjectFileManager *importer)
    : KJob(0), d(new ImportProjectJobPrivate )
{
    d->m_importer = importer;
    d->m_folder = folder;
    d->m_project = folder->project();

    setObjectName(i18n("Project Import: %1", d->m_project->name()));
    connect(ICore::self(), &ICore::aboutToShutdown,
            this, &ImportProjectJob::aboutToShutdown);
}

ImportProjectJob::~ImportProjectJob()
{
    delete d;
}

void ImportProjectJob::start()
{
    d->m_watcher = new QFutureWatcher<void>();
    connect(d->m_watcher, &QFutureWatcher<void>::finished, this, &ImportProjectJob::importDone);
    connect(d->m_watcher, &QFutureWatcher<void>::canceled, this, &ImportProjectJob::importCanceled);
    QFuture<void> f = QtConcurrent::run(d, &ImportProjectJobPrivate::import, d->m_folder);
    d->m_watcher->setFuture(f);
}

void ImportProjectJob::importDone()
{
    d->m_watcher->deleteLater(); /* Goodbye to the QFutureWatcher */

    emitResult();
}

bool ImportProjectJob::doKill()
{
    d->m_watcher->cancel();
    d->cancel=true;

    setError(1);
    setErrorText(i18n("Project import canceled."));

    d->m_watcher->waitForFinished();
    return true;
}

void ImportProjectJob::aboutToShutdown()
{
    kill();
}

void ImportProjectJob::importCanceled()
{
    d->m_watcher->deleteLater();
}

}


