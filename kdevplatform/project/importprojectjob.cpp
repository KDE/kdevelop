/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "importprojectjob.h"
#include "projectmodel.h"

#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QPointer>

#include <interfaces/iprojectfilemanager.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>

#include <KLocalizedString>

namespace KDevelop
{

class ImportProjectJobPrivate
{
public:
    ImportProjectJobPrivate() {}
    ProjectFolderItem *m_folder;
    IProjectFileManager *m_importer;
    QFutureWatcher<void> *m_watcher;
    QPointer<IProject> m_project;
    bool cancel = false;

    void import(ProjectFolderItem* folder)
    {
        const auto subs = m_importer->parse(folder);
        for (ProjectFolderItem* sub : subs) {
            if(!cancel) {
                import(sub);
            }
        }
    }
};

ImportProjectJob::ImportProjectJob(ProjectFolderItem *folder, IProjectFileManager *importer)
    : KJob(nullptr), d(new ImportProjectJobPrivate )
{
    setCapabilities(Killable);

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
    QFuture<void> f = QtConcurrent::run([this] {d->import(d->m_folder);});
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

#include "moc_importprojectjob.cpp"
