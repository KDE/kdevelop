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
#include "util/debug.h"

#include "interfaces/icore.h"
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include "language/codegen/documentchangeset.h"
#include "codedescription.h"
#include "templaterenderer.h"
#include "sourcefiletemplate.h"
#include "templateengine.h"
#include <duchain/declaration.h>
#include <duchain/duchainlock.h>
#include <duchain/persistentsymboltable.h>
#include <duchain/types/structuretype.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <QFileInfo>

using namespace KDevelop;

/// @param base String such as 'public QObject' or 'QObject'
InheritanceDescription descriptionFromString(const QString& base)
{
    QStringList splitBase = base.split(' ');
    QString identifier = splitBase.takeLast();
    QString inheritanceMode = splitBase.join(" ");

    InheritanceDescription desc;
    desc.baseType = identifier;
    desc.inheritanceMode = inheritanceMode;
    return desc;
}

class KDevelop::TemplateClassGeneratorPrivate
{
public:
    SourceFileTemplate fileTemplate;
    QUrl baseUrl;
    TemplateRenderer renderer;

    QString name;
    QString identifier;
    QStringList namespaces;
    QString license;

    QHash<QString, QUrl> fileUrls;
    QHash<QString, KTextEditor::Cursor> filePositions;
    ClassDescription description;

    QList<DeclarationPointer> directBaseClasses;
    QList<DeclarationPointer> allBaseClasses;

    void fetchSuperClasses(const DeclarationPointer& declaration);
};

void TemplateClassGeneratorPrivate::fetchSuperClasses(const DeclarationPointer& declaration)
{
    DUChainReadLocker lock;

    //Prevent duplicity
    if(allBaseClasses.contains(declaration))
    {
        return;
    }

    allBaseClasses << declaration;

    DUContext* context = declaration->internalContext();
    if (context) {
        foreach (const DUContext::Import& import, context->importedParentContexts()) {
            if (DUContext * parentContext = import.context(context->topContext())) {
                if (parentContext->type() == DUContext::Class) {
                    fetchSuperClasses( DeclarationPointer(parentContext->owner()) );
                }
            }
        }
    }
}


TemplateClassGenerator::TemplateClassGenerator(const QUrl& baseUrl)
 : d(new TemplateClassGeneratorPrivate)
{
    Q_ASSERT(baseUrl.path().endsWith('/')); // assume folder

    d->baseUrl = baseUrl;
    d->renderer.setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
}

TemplateClassGenerator::~TemplateClassGenerator()
{
    delete d;
}

void TemplateClassGenerator::setTemplateDescription(const SourceFileTemplate& fileTemplate)
{
    d->fileTemplate = fileTemplate;
    Q_ASSERT(fileTemplate.isValid());
}

DocumentChangeSet TemplateClassGenerator::generate()
{
    return d->renderer.renderFileTemplate(d->fileTemplate, d->baseUrl, fileUrls());
}

QHash<QString, QString> TemplateClassGenerator::fileLabels() const
{
    Q_ASSERT(d->fileTemplate.isValid());
    QHash<QString,QString> labels;

    foreach (const SourceFileTemplate::OutputFile& outputFile, d->fileTemplate.outputFiles())
    {
        labels.insert(outputFile.identifier, outputFile.label);
    }

    return labels;
}

TemplateClassGenerator::UrlHash TemplateClassGenerator::fileUrls() const
{
    if (d->fileUrls.isEmpty())
    {
        foreach (const SourceFileTemplate::OutputFile& outputFile, d->fileTemplate.outputFiles())
        {
            QString outputName = d->renderer.render(outputFile.outputName, outputFile.identifier);
            QUrl url = d->baseUrl.resolved(QUrl(outputName));
            d->fileUrls.insert(outputFile.identifier, url);
        }
    }

    return d->fileUrls;
}

QUrl TemplateClassGenerator::baseUrl() const
{
    return d->baseUrl;
}

QUrl TemplateClassGenerator::fileUrl(const QString& outputFile) const
{
    return fileUrls().value(outputFile);
}

void TemplateClassGenerator::setFileUrl(const QString& outputFile, const QUrl& url)
{
    d->fileUrls.insert(outputFile, url);
    d->renderer.addVariable("output_file_" + outputFile.toLower(), QDir(d->baseUrl.path()).relativeFilePath(url.path()));
    d->renderer.addVariable("output_file_" + outputFile.toLower() + "_absolute", url.toLocalFile());
}

KTextEditor::Cursor TemplateClassGenerator::filePosition(const QString& outputFile) const
{
    return d->filePositions.value(outputFile);
}

void TemplateClassGenerator::setFilePosition(const QString& outputFile, const KTextEditor::Cursor& position)
{
    d->filePositions.insert(outputFile, position);
}

void TemplateClassGenerator::addVariables(const QVariantHash& variables)
{
    d->renderer.addVariables(variables);
}

QString TemplateClassGenerator::renderString(const QString& text) const
{
    return d->renderer.render(text);
}

SourceFileTemplate TemplateClassGenerator::sourceFileTemplate() const
{
    return d->fileTemplate;
}

TemplateRenderer* TemplateClassGenerator::renderer() const
{
    return &(d->renderer);
}


QString TemplateClassGenerator::name() const
{
    return d->name;
}

void TemplateClassGenerator::setName(const QString& newName)
{
    d->name = newName;
    d->renderer.addVariable("name", newName);
}

QString TemplateClassGenerator::identifier() const
{
    return name();
}

void TemplateClassGenerator::setIdentifier(const QString& identifier)
{
    d->renderer.addVariable("identifier", identifier);;
    QStringList separators;
    separators << "::" << "." << ":" << "\\" << "/";
    QStringList ns;
    foreach (const QString& separator, separators)
    {
        ns = identifier.split(separator);
        if (ns.size() > 1)
        {
            break;
        }
    }
    setName(ns.takeLast());
    setNamespaces(ns);
}

QStringList TemplateClassGenerator::namespaces() const
{
    return d->namespaces;
}

void TemplateClassGenerator::setNamespaces(const QStringList& namespaces) const
{
    d->namespaces = namespaces;
    d->renderer.addVariable("namespaces", namespaces);
}


/// Specify license for this class
void TemplateClassGenerator::setLicense(const QString& license)
{
    qCDebug(LANGUAGE) << "New Class: " << d->name << "Set license: " << d->license;
    d->license = license;
    d->renderer.addVariable("license", license);
}

/// Get the license specified for this classes
QString TemplateClassGenerator::license() const
{
    return d->license;
}

void TemplateClassGenerator::setDescription(const ClassDescription& description)
{
    d->description = description;

    QVariantHash variables;
    variables["description"] = QVariant::fromValue(description);
    variables["members"] = CodeDescription::toVariantList(description.members);
    variables["functions"] = CodeDescription::toVariantList(description.methods);
    variables["base_classes"] = CodeDescription::toVariantList(description.baseClasses);
    d->renderer.addVariables(variables);
}

ClassDescription TemplateClassGenerator::description() const
{
    return d->description;
}

void TemplateClassGenerator::addBaseClass(const QString& base)
{
    const InheritanceDescription desc = descriptionFromString(base);

    ClassDescription cd = description();
    cd.baseClasses << desc;
    setDescription(cd);

    DUChainReadLocker lock;

    PersistentSymbolTable::Declarations decl = PersistentSymbolTable::self().getDeclarations(IndexedQualifiedIdentifier(QualifiedIdentifier(desc.baseType)));

    //Search for all super classes
    for(PersistentSymbolTable::Declarations::Iterator it = decl.iterator(); it; ++it)
    {
        DeclarationPointer declaration = DeclarationPointer(it->declaration());
        if(declaration->isForwardDeclaration())
        {
            continue;
        }

        // Check if it's a class/struct/etc
        if(declaration->type<StructureType>())
        {
            d->fetchSuperClasses(declaration);
            d->directBaseClasses << declaration;
            break;
        }
    }
}

void TemplateClassGenerator::setBaseClasses(const QList<QString>& bases)
{
    // clear
    ClassDescription cd = description();
    cd.baseClasses.clear();
    setDescription(cd);

    d->directBaseClasses.clear();
    d->allBaseClasses.clear();

    // add all bases
    foreach (const QString& base, bases) {
        addBaseClass(base);
    }
}

QList< DeclarationPointer > TemplateClassGenerator::directBaseClasses() const
{
    return d->directBaseClasses;
}

QList< DeclarationPointer > TemplateClassGenerator::allBaseClasses() const
{
    return d->allBaseClasses;
}

