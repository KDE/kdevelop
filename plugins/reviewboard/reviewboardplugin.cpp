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
#include <QFile>
#include <interfaces/icore.h>
#include <interfaces/ipatchsource.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/vcsjob.h>
#include "reviewpatchdialog.h"
#include "reviewboardjobs.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevReviewBoardFactory, registerPlugin<ReviewBoardPlugin>(); )
K_EXPORT_PLUGIN(KDevReviewBoardFactory(KAboutData("kdevreviewboard","kdevreviewboard", ki18n("ReviewBoard Support"), "0.1", ki18n("Deal with the ReviewBoard Patches"), KAboutData::License_GPL)))

ReviewBoardPlugin::ReviewBoardPlugin ( QObject* parent, const QVariantList& ) 
    : IPlugin ( KDevReviewBoardFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IPatchExporter )
}

ReviewBoardPlugin::~ReviewBoardPlugin()
{}

QByteArray urlToData(const KUrl& url)
{
    QByteArray ret;
    if(url.isLocalFile()) {
        QFile f(url.toLocalFile());
        Q_ASSERT(f.exists());
        bool corr=f.open(QFile::ReadOnly | QFile::Text);
        Q_ASSERT(corr);
        
        ret = f.readAll();
        
    } else {
#if defined(__GNUC__)
#warning TODO: add downloading the data
#endif
    }
    return ret;
}

void ReviewBoardPlugin::exportPatch(IPatchSource::Ptr source)
{
    ReviewPatchDialog d;
   
    IProject* p = ICore::self()->projectController()->findProjectForUrl(source->baseDir());
    
    if(p) {
        KConfigGroup versionedConfig = p->projectConfiguration()->group("ReviewBoard");
    
        d.setServer(versionedConfig.readEntry<KUrl>("server", KUrl("https://git.reviewboard.kde.org")));
        d.setUsername(versionedConfig.readEntry("username", QString()));
    }
    
    int ret = d.exec();
    if(ret==KDialog::Accepted) {
        ReviewBoard::NewRequest* job=new ReviewBoard::NewRequest(d.server(), source->file(), d.repository(), d.baseDir());
        bool corr = job->exec();
        if(corr) {
            KUrl url=d.server();
            url.setUserInfo(QString());
            QString requrl = QString("%1/r/%2/").arg(url.prettyUrl()).arg(job->requestId());
            
            KMessageBox::information(0, i18n("<qt>You can find the new request at:<br /><a href='%1'>%1</a> </qt>", requrl));
        } else {
            KMessageBox::error(0, job->errorText());
        }
    }
}
