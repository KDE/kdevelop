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

#include "standardpatchexport.h"
#include <KFileDialog>
#include <KIO/CopyJob>
#include <KLocalizedString>
#include <interfaces/icore.h>
#include <interfaces/ipatchexporter.h>
#include <interfaces/iruncontroller.h>
#include <QMenu>
#include "patchreview.h"

class KIOExport : public KDevelop::IPatchExporter {
    virtual void exportPatch(KDevelop::IPatchSource::Ptr source)
    {
        KUrl dest=KFileDialog::getSaveUrl();
        if(!dest.isEmpty()) { //We let KDE do the rest of the job including the notification
            KIO::CopyJob* job=KIO::copy(source->file(), dest);
            KDevelop::ICore::self()->runController()->registerJob(job);
        }
    }
};

StandardPatchExport::StandardPatchExport(PatchReviewPlugin* plugin, QObject* parent)
    : QObject(parent), m_plugin(plugin)
{
    m_exporters.append(new KIOExport);
}

StandardPatchExport::~StandardPatchExport()
{
    qDeleteAll(m_exporters);
}

void StandardPatchExport::addActions(QMenu* m)
{
    m->addAction(KIcon("document-save"), i18n("Save As..."), this, SLOT(runKIOExport()));
}

void StandardPatchExport::runKIOExport()
{
    m_exporters[0]->exportPatch(m_plugin->patch());
}
