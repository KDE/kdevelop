/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sourcefiletemplate.h"

#include <interfaces/icore.h>

#include <KDebug>
#include <KComponentData>
#include <KStandardDirs>
#include <KArchive>
#include <KZip>
#include <KTar>
#include <KConfigGroup>

#include <QFileInfo>

using namespace KDevelop;

class KDevelop::SourceFileTemplatePrivate
{
public:
    KArchive* archive;
    QString descriptionFileName;
};

SourceFileTemplate::SourceFileTemplate(const QString& templateDescription)
: d(new SourceFileTemplatePrivate)
{
    d->descriptionFileName = templateDescription;
    QString archiveFileName;

    foreach (const QString& file, ICore::self()->componentData().dirs()->findAllResources("filetemplates"))
    {
        kDebug() << "Found template archive" << file;
        if (QFileInfo(file).baseName() == QFileInfo(templateDescription).baseName())
        {
            archiveFileName = file;
            break;
        }
    }

    if (archiveFileName.isEmpty())
    {
        kDebug() << "Could not find a template archive for description" << templateDescription;
    }
    else
    {
        QFileInfo info(archiveFileName);

        if (info.suffix() == ".zip")
        {
            d->archive = new KZip(archiveFileName);
        }
        else
        {
            d->archive = new KTar(archiveFileName);
        }
        d->archive->open(QIODevice::ReadOnly);
    }
}

SourceFileTemplate::~SourceFileTemplate()
{
    delete d->archive;
    delete d;
}

const KArchiveDirectory* SourceFileTemplate::directory()
{
    return d->archive->directory();
}

QList< SourceFileTemplate::OutputFile > SourceFileTemplate::outputFiles()
{
    QList<SourceFileTemplate::OutputFile> outputFiles;
    
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup group(&templateConfig, "General");

    QStringList files = group.readEntry("Files", QStringList());
    kDebug() << "Files in template" << files;
    foreach (const QString& fileGroup, files)
    {
        KConfigGroup cg(&templateConfig, fileGroup);
        OutputFile f;
        f.identifier = cg.name();
        f.label = cg.readEntry("Name");
        f.fileName = cg.readEntry("File");
        f.outputName = cg.readEntry("OutputFile");
        outputFiles << f;
    }

    return outputFiles;
}

bool SourceFileTemplate::hasCustomOptions()
{
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    bool hasOptions = d->archive->directory()->entries().contains(cg.readEntry("OptionsFile", "options.kcfg"));

    kDebug() << cg.readEntry("OptionsFile", "options.kcfg") << hasOptions;
    return hasOptions;
}

QByteArray SourceFileTemplate::customOptions()
{
    KConfig templateConfig(d->descriptionFileName);
    KConfigGroup cg(&templateConfig, "General");
    const KArchiveEntry* entry = d->archive->directory()->entry(cg.readEntry("OptionsFile", "options.kcfg"));

    if (entry->isFile())
    {
        const KArchiveFile* file = static_cast<const KArchiveFile*>(entry);
        return file->data();
    }

    return QByteArray();
}
