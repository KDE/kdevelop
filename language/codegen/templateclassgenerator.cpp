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
#include "sourcefiletemplate.h"

#include <KArchive>
#include <KZip>
#include <KComponentData>
#include <KStandardDirs>
#include <KTar>

#include <grantlee/engine.h>
#include <grantlee/metatype.h>

#include <QFileInfo>

using namespace KDevelop;

class KDevelop::TemplateClassGeneratorPrivate
{
public:
    SourceFileTemplate* fileTemplate;
    KUrl baseUrl;
    TemplateRenderer renderer;
};

TemplateClassGenerator::TemplateClassGenerator(const KUrl& baseUrl) : ClassGenerator(),
d(new TemplateClassGeneratorPrivate)
{
    d->fileTemplate = 0;
    d->baseUrl = baseUrl;
    d->renderer.setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
}

TemplateClassGenerator::~TemplateClassGenerator()
{
    delete d->fileTemplate;
    delete d;
}

void TemplateClassGenerator::setTemplateDescription (const QString& templateDescription)
{
    d->fileTemplate = new SourceFileTemplate(templateDescription);
    d->renderer.addArchive(d->fileTemplate->directory());
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

    return d->renderer.renderFileTemplate(d->fileTemplate, fileUrls());
}

QHash< QString, QString > TemplateClassGenerator::fileLabels()
{
    Q_ASSERT(d->fileTemplate);
    QHash<QString,QString> labels;

    foreach (const SourceFileTemplate::OutputFile& outputFile, d->fileTemplate->outputFiles())
    {
        labels.insert(outputFile.identifier, outputFile.label);
    }

    return labels;
}

QHash< QString, KUrl > TemplateClassGenerator::fileUrlsFromBase (const KUrl& baseUrl, bool toLower)
{
    QHash<QString, KUrl> map;

    d->renderer.addVariables(templateVariables());

    foreach (const SourceFileTemplate::OutputFile& outputFile, d->fileTemplate->outputFiles())
    {
        KUrl url(baseUrl);
        QString outputName = d->renderer.render(outputFile.outputName, outputFile.identifier);
        if (toLower)
        {
            outputName = outputName.toLower();
        }
        url.addPath(outputName);
        map.insert(outputFile.identifier, url);
    }

    return map;
}

void TemplateClassGenerator::addVariables(const QVariantHash& variables)
{
    d->renderer.addVariables(variables);
}

QString TemplateClassGenerator::renderString (const QString& text)
{
    return d->renderer.render(text);
}

SourceFileTemplate* TemplateClassGenerator::sourceFileTemplate()
{
    return d->fileTemplate;
}
