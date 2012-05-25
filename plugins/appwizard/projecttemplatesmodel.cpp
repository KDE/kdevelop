/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "projecttemplatesmodel.h"

#include <QFileInfo>
#include <QDir>

#ifdef Q_WS_WIN
#include <kzip.h>
#else
#include <ktar.h>
#endif //Q_WS_WIN
#include <kurl.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "appwizardplugin.h"
#include <interfaces/icore.h>
#include <KIcon>
#include <KMimeType>

ProjectTemplatesModel::ProjectTemplatesModel(AppWizardPlugin *parent)
    :KDevelop::TemplatesModel(parent)
{
    setDescriptionResourceType("apptemplate_descriptions");
    setTemplateResourceType("apptemplates");
}

void ProjectTemplatesModel::refresh()
{
    KDevelop::TemplatesModel::refresh();
    setHorizontalHeaderLabels(QStringList() << i18n("Project Templates"));    
}

QString ProjectTemplatesModel::loadTemplateFile (const QString& fileName)
{
    QString saveLocation = KDevelop::ICore::self()->componentData().dirs()->saveLocation("apptemplates");
    QFileInfo info(fileName);
    QString destination = saveLocation + info.baseName();

    KMimeType::Ptr mimeType = KMimeType::findByUrl(KUrl(fileName));
    kDebug() << "Loaded file" << fileName << "with type" << mimeType->name();

    if (mimeType->name() == "application/x-desktop")
    {
        kDebug() << "Loaded desktop file" << info.absoluteFilePath() << ", compressing";
#ifdef Q_WS_WIN
        destination += ".zip";
        KZip archive(destination);
#else
        destination += ".tar.bz2";
        KTar archive(destination, "application/x-bzip");
#endif //Q_WS_WIN

        archive.open(QIODevice::WriteOnly);

        QDir dir(info.absoluteDir());
        QDir::Filters filter = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
        foreach (const QFileInfo& entry, dir.entryInfoList(filter))
        {
            if (entry.isFile())
            {
                archive.addLocalFile(entry.absoluteFilePath(), entry.fileName());
            }
            else if (entry.isDir())
            {
                archive.addLocalDirectory(entry.absoluteFilePath(), entry.fileName());
            }
        }
        archive.close();
    }
    else
    {
        kDebug() << "Copying" << fileName << "to" << saveLocation;
        QFile::copy(fileName, destination);
    }

    refresh();
    
    return destination;
}

