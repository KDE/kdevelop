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
#include "interfaces/icore.h"
#include "language/codegen/documentchangeset.h"

#include <KArchive>
#include <KZip>
#include <KComponentData>
#include <KStandardDirs>
#include <KTar>

#include <grantlee/engine.h>

#include <QFileInfo>

using namespace KDevelop;

class KDevelop::TemplateClassGeneratorPrivate
{
public:
    QString templateDescription;
    KArchive* archive;
    KUrl baseUrl;
    
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
}

TemplateClassGenerator::TemplateClassGenerator(const KUrl& baseUrl) : ClassGenerator(),
d(new TemplateClassGeneratorPrivate)
{
    d->archive = 0;
    d->baseUrl = baseUrl;
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

DocumentChangeSet TemplateClassGenerator::generate()
{
    QVariantHash variables;
    
    variables["name"] = name();
    variables["identifier"] = identifier();
    variables["license"] = license();
    
    DocumentChangeSet changes;
    
    Grantlee::Engine engine;
    Grantlee::Context context(variables);
    
    // TODO: Add more variables to context
    
    d->archive->open(QIODevice::ReadOnly);
    const KArchiveDirectory* dir = d->archive->directory();
    
    kDebug() << "Opened archive with contents:" << dir->entries();
    
    KConfig templateConfig(d->templateDescription);
    foreach (const QString& groupName, templateConfig.groupList())
    {
        if (groupName == "General")
        {
            continue;
        }
        
        KConfigGroup cg(&templateConfig, groupName);
        const KArchiveEntry* entry = dir->entry(cg.readEntry("File"));
        if (!entry)
        {
            kDebug() << "Entry" << cg.readEntry("File") << "is mentioned in group" << cg.name() << "but is not present in the archive";
            continue;
        }
        
        const KArchiveFile* file = dynamic_cast<const KArchiveFile*>(entry);
        if (!file)
        {
            kDebug() << "Entry" << cg.readEntry("File") << "is not a file";
            continue;
        }
        
        Grantlee::Template nameTemplate = engine.newTemplate(cg.readEntry("OutputFile"), cg.name());
        QString outputName = nameTemplate->render(&context);
        
        KUrl url(d->baseUrl);
        url.addPath(outputName);
        IndexedString document(url);
        SimpleRange range(SimpleCursor(0, 0), 0);
        
        Grantlee::Template fileTemplate = engine.newTemplate(file->data(), outputName);

        DocumentChange change(document, range, QString(), fileTemplate->render(&context));
        changes.addChange(change);
    }

    return changes;
}

QMap< QString, KUrl > TemplateClassGenerator::fileUrlsFromBase (const KUrl& baseUrl, bool toLower)
{
    QMap<QString, KUrl> map;
    
    Grantlee::Engine engine;
    Grantlee::Context context;
  
    KConfig templateConfig(d->templateDescription);
    foreach (const QString& groupName, templateConfig.groupList())
    {
        if (groupName == "General")
        {
            continue;
        }
        
        KConfigGroup cg(&templateConfig, groupName);
        
        Grantlee::Template nameTemplate = engine.newTemplate(cg.readEntry("OutputFile"), cg.name());
        
        KUrl url(baseUrl);
        url.addPath(nameTemplate->render(&context));
        QString fileName = cg.readEntry("Name");
        if (toLower)
        {
            fileName = fileName.toLower();
        }
        map.insert(fileName, url);
    }
    
    return map;
}
