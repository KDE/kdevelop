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
    QString templateName;
    KArchive* archive;
    
    void loadTemplate();
};

void TemplateClassGeneratorPrivate::loadTemplate()
{    
    QString fileName = ICore::self()->componentData().dirs()->findResource("kdevfiletemplates", templateName);
    QFileInfo info(fileName);
    
    if (info.suffix() == ".zip")
    {
        archive = new KZip(fileName);
    }
    else
    {
        archive = new KTar(fileName);
    }
}

TemplateClassGenerator::TemplateClassGenerator(const QString& templateName) : ClassGenerator(),
d(new TemplateClassGeneratorPrivate)
{
    d->templateName = templateName;
    d->loadTemplate();
}

TemplateClassGenerator::~TemplateClassGenerator()
{
    delete d->archive;
    delete d;
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
    foreach (const QString& entryName, dir->entries())
    {
        const KArchiveFile* file = dynamic_cast<const KArchiveFile*>(dir->entry(entryName));
        if (!file || file->name() == d->templateName + ".desktop")
        {
            continue;
        }
        
        QString outputName = KMacroExpander::expandMacros(file->name(), variables);
        
        Grantlee::Template t = engine.newTemplate(file->data(), outputName);
        
        // TODO: Map from outputName to the file type
        // Preferably using the .desktop file
        
        QFile outputFile(outputName);
        if (!outputFile.open(QIODevice::WriteOnly))
        {
            continue;
        }
        
        outputFile.write(t->render(&context).toUtf8());
    }
    
    return DocumentChangeSet();
}

QMap< QString, KUrl > TemplateClassGenerator::fileUrlsFromBase (const KUrl& baseUrl, bool toLower)
{
    QMap<QString, KUrl> map;
    
    // TODO: Lookup files from the template archive
    return map;
}
