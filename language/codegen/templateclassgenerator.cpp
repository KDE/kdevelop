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
#include <KMacroExpanderBase>

#include <grantlee/engine.h>

#include <QFileInfo>

using namespace KDevelop;

class KDevelop::TemplateClassGeneratorPrivate
{
public:
    QString templateDescription;
    KArchive* archive;
    
    void loadTemplate();
};

void TemplateClassGeneratorPrivate::loadTemplate()
{    
    QString archiveFileName;
    
    foreach (const QString& file, ICore::self()->componentData().dirs()->findAllResources("kdevfiletemplates"))
    {
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

TemplateClassGenerator::TemplateClassGenerator() : ClassGenerator(),
d(new TemplateClassGeneratorPrivate)
{
    d->archive = 0;
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
    QHash<QString, QString> variables;
    
    variables["class_name"] = name();
    variables["identifier"] = identifier();
    
    DocumentChangeSet changes;
    
    Grantlee::Engine engine;
    Grantlee::Context context;
    
    // TODO: Add variables to context
    
    const KArchiveDirectory* dir = d->archive->directory();
    
    KConfig templateConfig(d->templateDescription);
    foreach (const QString& groupName, templateConfig.groupList())
    {
        if (groupName == "General")
        {
            continue;
        }
        
        KConfigGroup cg(&templateConfig, groupName);
        const KArchiveFile* file = dynamic_cast<const KArchiveFile*>(dir->entry(cg.readEntry("File")));
        if (!file)
        {
            kDebug() << "File" << cg.readEntry("File") << "is mentioned in the description, but not present in the archive";
            continue;
        }
        
        Grantlee::Template nameTemplate = engine.newTemplate(cg.readEntry("OutputFile"), cg.name());
        QString outputName = nameTemplate->render(&context);
        
        Grantlee::Template fileTemplate = engine.newTemplate(file->data(), outputName);
        
        QFile outputFile(outputName);
        if (!outputFile.open(QIODevice::WriteOnly))
        {
            continue;
        }
        
        outputFile.write(fileTemplate->render(&context).toUtf8());
        outputFile.close();
    }
    
    return DocumentChangeSet();
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
        QString outputName = nameTemplate->render(&context);
        
        map.insert(cg.readEntry("Name"), outputName);
    }
    
    return map;
}
