/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#include "templateclassgenerator.h"
#include "archivetemplateloader.h"

#include "interfaces/icore.h"
#include "language/codegen/documentchangeset.h"
#include "codedescription.h"
#include "templaterenderer.h"

#include <KArchive>
#include <KZip>
#include <KComponentData>
#include <KStandardDirs>
#include <KTar>
#include <KConfigGroup>

#include <grantlee/engine.h>
#include <grantlee/metatype.h>

#include <QFileInfo>

using namespace KDevelop;

class KDevelop::TemplateClassGeneratorPrivate
{
public:
    QString templateDescription;
    KArchive* archive;
    KUrl baseUrl;
    TemplateRenderer renderer;

    void loadTemplate();
};

void TemplateClassGeneratorPrivate::loadTemplate()
{
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
        return;
    }

    QFileInfo info(archiveFileName);

    if (info.suffix() == ".zip")
    {
        archive = new KZip(archiveFileName);
    }
    else
    {
        archive = new KTar(archiveFileName);
    }

    archive->open(QIODevice::ReadOnly);

    renderer.addArchive(archive->directory());
}

TemplateClassGenerator::TemplateClassGenerator(const KUrl& baseUrl) : ClassGenerator(),
d(new TemplateClassGeneratorPrivate)
{
    d->archive = 0;
    d->baseUrl = baseUrl;
    d->renderer.setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
}

TemplateClassGenerator::~TemplateClassGenerator()
{
    delete d->archive;
    delete d;
}

void TemplateClassGenerator::setTemplateDescription (const QString& templateDescription)
{
    d->templateDescription = templateDescription;
    d->loadTemplate();
}

QVariantHash TemplateClassGenerator::templateVariables()
{
    QVariantHash variables;

    variables["name"] = name();
    variables["identifier"] = identifier();
    variables["license"] = license();

    ClassDescription desc = description();
    variables["description"] = QVariant::fromValue(desc);
    variables["members"] = CodeDescription::toVariantList(desc.members);
    variables["functions"] = CodeDescription::toVariantList(desc.methods);
    variables["base_classes"] = CodeDescription::toVariantList(desc.baseClasses);

    d->baseUrl.adjustPath(KUrl::AddTrailingSlash);
    QHash<QString,KUrl> urls = fileUrls();
    QRegExp nonAlphaNumeric("\\W");
    for (QHash<QString,KUrl>::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        QString cleanName = it.key().toLower();
        cleanName.replace(nonAlphaNumeric, "_");
        variables["output_file_" + cleanName] = KUrl::relativeUrl(d->baseUrl, it.value());
        variables["output_file_" + cleanName + "_absolute"] = it.value().toLocalFile();
    }

    return variables;
}


DocumentChangeSet TemplateClassGenerator::generate()
{
    Q_ASSERT(d->archive);
    d->renderer.addVariables(templateVariables());

    DocumentChangeSet changes;

    const KArchiveDirectory* dir = d->archive->directory();

    KConfig templateConfig(d->templateDescription);
    QHash<QString,KUrl> urls = fileUrls();

    for (QHash<QString,KUrl>::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        KConfigGroup cg(&templateConfig, it.key());
        QString fileName = cg.readEntry("File");

        if (fileName.isEmpty())
        {
            continue;
        }

        const KArchiveEntry* entry = dir->entry(fileName);
        if (!entry)
        {
            kDebug() << "Entry" << cg.readEntry("File") << "is mentioned in group" << cg.name() << "but is not present in the archive";
            continue;
        }

        const KArchiveFile* file = dynamic_cast<const KArchiveFile*>(entry);
        if (!file)
        {
            kDebug() << "Entry" << entry->name() << "is not a file";
            continue;
        }

        KUrl url = it.value();
        IndexedString document(url);
        SimpleRange range(SimpleCursor(0, 0), 0);

        DocumentChange change(document, range, QString(), d->renderer.render(file->data(), it.key()));
        changes.addChange(change);
        kDebug() << "Added change for file" << document.str();
    }

    return changes;
}

QHash< QString, QString > TemplateClassGenerator::fileLabels()
{
    Q_ASSERT(!d->templateDescription.isEmpty());
    QHash<QString,QString> labels;

    KConfig templateConfig(d->templateDescription);
    KConfigGroup group(&templateConfig, "General");

    QStringList files = group.readEntry("Files", QStringList());
    kDebug() << "Files in template" << files;
    foreach (const QString& fileGroup, files)
    {
        KConfigGroup cg(&templateConfig, fileGroup);
        if (cg.hasKey("OutputFile"))
        {
            labels.insert(cg.name(), cg.readEntry("Name"));
        }
    }

    return labels;
}

QHash< QString, KUrl > TemplateClassGenerator::fileUrlsFromBase (const KUrl& baseUrl, bool toLower)
{
    QHash<QString, KUrl> map;

    d->renderer.addVariables(templateVariables());

    KConfig templateConfig(d->templateDescription);
    KConfigGroup group(&templateConfig, "General");
    QStringList files = group.readEntry("Files", QStringList());
    kDebug() << "Files in template" << files;

    foreach (const QString& fileGroup, files)
    {
        KConfigGroup cg(&templateConfig, fileGroup);
        if (!cg.hasKey("OutputFile"))
        {
            continue;
        }

        KUrl url(baseUrl);
        QString outputName = d->renderer.render(cg.readEntry("OutputFile"), cg.name());
        if (toLower)
        {
            outputName = outputName.toLower();
        }
        url.addPath(outputName);

        QString fileType = cg.name();
        map.insert(fileType, url);
    }

    return map;
}

bool TemplateClassGenerator::hasCustomOptions()
{
    KConfig templateConfig(d->templateDescription);
    KConfigGroup cg(&templateConfig, "General");
    bool hasOptions = d->archive->directory()->entries().contains(cg.readEntry("OptionsFile", "options.kcfg"));

    kDebug() << cg.readEntry("OptionsFile", "options.kcfg") << hasOptions;
    return hasOptions;
}

QByteArray TemplateClassGenerator::customOptions()
{
    KConfig templateConfig(d->templateDescription);
    KConfigGroup cg(&templateConfig, "General");
    const KArchiveEntry* entry = d->archive->directory()->entry(cg.readEntry("OptionsFile", "options.kcfg"));

    if (entry->isFile())
    {
        const KArchiveFile* file = dynamic_cast<const KArchiveFile*>(entry);
        return file->data();
    }

    return QByteArray();
}

void TemplateClassGenerator::addVariables(const QVariantHash& variables)
{
    d->renderer.addVariables(variables);
}

QString TemplateClassGenerator::renderString (const QString& text)
{
    return d->renderer.render(text);
}
