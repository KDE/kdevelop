/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "reviewboardplugin.h"
#include <QVariantList>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KLocale>
#include <KDialog>
#include <KMessageBox>
#include <KDebug>
#include <KIO/Job>
#include <KUrl>
#include <QFile>
#include <QDir>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/interfaces/ipatchsource.h>
#include <vcs/vcsjob.h>
#include "reviewpatchdialog.h"
#include "reviewboardjobs.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevReviewBoardFactory, registerPlugin<ReviewBoardPlugin>(); )
// K_EXPORT_PLUGIN(KDevReviewBoardFactory(KAboutData("kdevreviewboard","kdevreviewboard", ki18n("ReviewBoard Support"), "0.1", ki18n("Deal with the ReviewBoard Patches"), KAboutData::License_GPL)))

ReviewBoardPlugin::ReviewBoardPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( "kdevreviewboard", parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IPatchExporter )
}

ReviewBoardPlugin::~ReviewBoardPlugin()
{}

void ReviewBoardPlugin::exportPatch(IPatchSource::Ptr source)
{
    KUrl dirUrl = source->baseDir();
    m_source = source;
    ReviewPatchDialog d(dirUrl);

    dirUrl.adjustPath(KUrl::RemoveTrailingSlash);
    IProject* p = ICore::self()->projectController()->findProjectForUrl(dirUrl);

    if(p) {
        KConfigGroup versionedConfig = p->projectConfiguration()->group("ReviewBoard");

        if(versionedConfig.hasKey("server")) d.setServer(versionedConfig.readEntry<QUrl>("server", QUrl()));
        if(versionedConfig.hasKey("username")) d.setUsername(versionedConfig.readEntry("username", QString()));
        if(versionedConfig.hasKey("baseDir")) d.setBaseDir(versionedConfig.readEntry("baseDir", "/"));
        if(versionedConfig.hasKey("repository")) d.setRepository(versionedConfig.readEntry("repository", QString()));
    }

    int ret = d.exec();
    if(ret==KDialog::Accepted) {
        KJob* job;
        if (d.isUpdateReview()) {
            job=new ReviewBoard::SubmitPatchRequest(d.server(), source->file(), d.baseDir(), d.review());
            connect(job, SIGNAL(finished(KJob*)), SLOT(reviewDone(KJob*)));
        } else {
            m_baseDir = d.baseDir();
            job=new ReviewBoard::NewRequest(d.server(), d.repository());
            connect(job, SIGNAL(finished(KJob*)), SLOT(reviewCreated(KJob*)));
        }

        job->start();

        if(p) {
            KConfigGroup versionedConfig = p->projectConfiguration()->group("ReviewBoard");

            // We store username in a diferent field. Unset it from server.
            QUrl storeServer(d.server());
            storeServer.setUserName(QString());
            // Don't store password in plaintext inside .kdev4
            storeServer.setPassword(QString());

            versionedConfig.writeEntry<QUrl>("server", storeServer);
            versionedConfig.writeEntry("username", d.username());
            versionedConfig.writeEntry("baseDir", d.baseDir());
            versionedConfig.writeEntry("repository", d.repository());
        }
    }
}

void ReviewBoardPlugin::reviewDone(KJob* j)
{
    if(j->error()==0) {
        ReviewBoard::SubmitPatchRequest const * job = qobject_cast<ReviewBoard::SubmitPatchRequest*>(j);
        KUrl url = job->server();
        url.setUserInfo(QString());
        QString requrl = QString("%1/r/%2/").arg(url.prettyUrl()).arg(job->requestId());

        KMessageBox::information(0, i18n("<qt>You can find the new request at:<br /><a href='%1'>%1</a> </qt>", requrl),
                                    QString(), QString(), KMessageBox::AllowLink);
    } else {
        KMessageBox::error(0, j->errorText());
    }
}

void ReviewBoardPlugin::reviewCreated(KJob* j)
{
    if (j->error()==0) {
        ReviewBoard::NewRequest const * job = qobject_cast<ReviewBoard::NewRequest*>(j);
        ReviewBoard::SubmitPatchRequest* submitPatchJob=new ReviewBoard::SubmitPatchRequest(job->server(), m_source->file(), m_baseDir, job->requestId());
        connect(submitPatchJob, SIGNAL(finished(KJob*)), SLOT(reviewDone(KJob*)));
        submitPatchJob->start();
    } else {
        KMessageBox::error(0, j->errorText());
    }
}

#include "reviewboardplugin.moc"
