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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <interfaces/ipatchsource.h>
#include <KIO/Job>
#include <KMessageBox>
#include <KDebug>
#include <QFile>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

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
#warning TODO: add downloading the data
    }
    return ret;
}

void ReviewBoardPlugin::exportPatch(IPatchSource::Ptr source)
{
}
