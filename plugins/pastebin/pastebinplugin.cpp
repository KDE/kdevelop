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

#include "pastebinplugin.h"
#include <QVariantList>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <interfaces/ipatchsource.h>
#include <KIO/Job>
#include <KMessageBox>
#include <QFile>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevPastebinFactory, registerPlugin<PastebinPlugin>(); )
K_EXPORT_PLUGIN(KDevPastebinFactory(KAboutData("kdevpastebin","kdevpastebin", ki18n("KDE Provider"), "0.1", ki18n("Easily import KDE projects"), KAboutData::License_GPL)))

PastebinPlugin::PastebinPlugin ( QObject* parent, const QVariantList& ) 
    : IPlugin ( KDevPastebinFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IPatchExporter )
}

PastebinPlugin::~PastebinPlugin()
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
#warning TODO: add downloading the patches from away
    }
    return ret;
}

void PastebinPlugin::exportPatch(IPatchSource* source)
{
    QByteArray bytearray = "paste_code="+QUrl::toPercentEncoding(urlToData(source->file()), "/");
    bytearray += "&paste_format=dff&paste_expiry_date=1D&paste_email=";

    KUrl url("http://pastebin.com/api_public.php");

    KIO::TransferJob *tf = KIO::http_post(url, bytearray, KIO::HideProgressInfo);

    tf->addMetaData("content-type","Content-Type: application/x-www-form-urlencoded");
    connect(tf, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(data(KIO::Job*, const QByteArray&)));
}

void PastebinPlugin::data(KIO::Job* job, const QByteArray &data)
{
    QMap< KIO::Job*, QString >::iterator it = m_result.find(job);
    Q_ASSERT(it!=m_result.end());
    if (data.isEmpty()) {
        if (job->error()) {
            KMessageBox::error(0, job->errorString());
        } else if (it->isEmpty() || it->startsWith("ERROR")) {
            KMessageBox::error(0, *it);
        } else {
            KMessageBox::information(0, *it);
        }
    } else {
        *it += data;
    }
}