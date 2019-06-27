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
#include <debug.h>

#include <interfaces/iproject.h>
#include "language/codegen/documentchangeset.h"
#include "codedescription.h"
#include "templaterenderer.h"
#include "sourcefiletemplate.h"
#include <duchain/declaration.h>
#include <duchain/duchainlock.h>
#include <duchain/persistentsymboltable.h>
#include <duchain/types/structuretype.h>
#include <project/projectmodel.h>

#include <QFileInfo>

using namespace KDevelop;

/// @param base String such as 'public QObject' or 'QObject'
InheritanceDescription descriptionFromString(const QString& base)
{
    QStringList splitBase = base.split(QLatin1Char(' '));
    QString identifier = splitBase.takeLast();
    QString inheritanceMode = splitBase.join(QLatin1Char(' '));

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
    // changes state when rendering
    mutable TemplateRenderer renderer;

    QString name;
    QString identifier;
    QStringList namespaces;
    QString license;

    // lazily estimated
    mutable QHash<QString, QUrl> fileUrls;
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
    if (allBaseClasses.contains(declaration)) {
        return;
    }

    allBaseClasses << declaration;

    DUContext* context = declaration->internalContext();
    if (context) {
        const auto importedParentContexts = context->importedParentContexts();
        for (const DUContext::Import& import : importedParentContexts) {
            if (DUContext* parentContext = import.context(context->topContext())) {
                if (parentContext->type() == DUContext::Class) {
                    fetchSuperClasses(DeclarationPointer(parentContext->owner()));
                }
            }
        }
    }
}

TemplateClassGenerator::TemplateClassGenerator(const QUrl& baseUrl)
    : d_ptr(new TemplateClassGeneratorPrivate)
{
    Q_D(TemplateClassGenerator);

    Q_ASSERT(QFileInfo(baseUrl.toLocalFile()).isDir()); // assume folder

    d->baseUrl = baseUrl;
    d->renderer.setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
}

TemplateClassGenerator::~TemplateClassGenerator() = default;

void TemplateClassGenerator::setTemplateDescription(const SourceFileTemplate& fileTemplate)
{
    Q_D(TemplateClassGenerator);

    d->fileTemplate = fileTemplate;
    Q_ASSERT(fileTemplate.isValid());
}

DocumentChangeSet TemplateClassGenerator::generate()
{
    Q_D(TemplateClassGenerator);

    return d->renderer.renderFileTemplate(d->fileTemplate, d->baseUrl, fileUrls());
}

QHash<QString, QString> TemplateClassGenerator::fileLabels() const
{
    Q_D(const TemplateClassGenerator);

    Q_ASSERT(d->fileTemplate.isValid());
    QHash<QString, QString> labels;

    const auto outputFiles = d->fileTemplate.outputFiles();
    labels.reserve(outputFiles.size());
    for (const SourceFileTemplate::OutputFile& outputFile : outputFiles) {
        labels.insert(outputFile.identifier, outputFile.label);
    }

    return labels;
}

TemplateClassGenerator::UrlHash TemplateClassGenerator::fileUrls() const
{
    Q_D(const TemplateClassGenerator);

    if (d->fileUrls.isEmpty()) {
        const auto outputFiles = d->fileTemplate.outputFiles();
        for (const SourceFileTemplate::OutputFile& outputFile : outputFiles) {
            QString outputName = d->renderer.render(outputFile.outputName, outputFile.identifier);
            QUrl url = d->baseUrl.resolved(QUrl(outputName));
            d->fileUrls.insert(outputFile.identifier, url);
        }
    }

    return d->fileUrls;
}

QUrl TemplateClassGenerator::baseUrl() const
{
    Q_D(const TemplateClassGenerator);

    return d->baseUrl;
}

QUrl TemplateClassGenerator::fileUrl(const QString& outputFile) const
{
    return fileUrls().value(outputFile);
}

void TemplateClassGenerator::setFileUrl(const QString& outputFile, const QUrl& url)
{
    Q_D(TemplateClassGenerator);

    d->fileUrls.insert(outputFile, url);
    d->renderer.addVariable(QLatin1String("output_file_") + outputFile.toLower(), QDir(
                                d->baseUrl.path()).relativeFilePath(url.path()));
    d->renderer.addVariable(QLatin1String("output_file_") + outputFile.toLower() + QLatin1String(
                                "_absolute"), url.toLocalFile());
}

KTextEditor::Cursor TemplateClassGenerator::filePosition(const QString& outputFile) const
{
    Q_D(const TemplateClassGenerator);

    return d->filePositions.value(outputFile);
}

void TemplateClassGenerator::setFilePosition(const QString& outputFile, const KTextEditor::Cursor& position)
{
    Q_D(TemplateClassGenerator);

    d->filePositions.insert(outputFile, position);
}

void TemplateClassGenerator::addVariables(const QVariantHash& variables)
{
    Q_D(TemplateClassGenerator);

    d->renderer.addVariables(variables);
}

QString TemplateClassGenerator::renderString(const QString& text) const
{
    Q_D(const TemplateClassGenerator);

    return d->renderer.render(text);
}

SourceFileTemplate TemplateClassGenerator::sourceFileTemplate() const
{
    Q_D(const TemplateClassGenerator);

    return d->fileTemplate;
}

TemplateRenderer* TemplateClassGenerator::renderer() const
{
    Q_D(const TemplateClassGenerator);

    return &(d->renderer);
}

QString TemplateClassGenerator::name() const
{
    Q_D(const TemplateClassGenerator);

    return d->name;
}

void TemplateClassGenerator::setName(const QString& newName)
{
    Q_D(TemplateClassGenerator);

    d->name = newName;
    d->renderer.addVariable(QStringLiteral("name"), newName);
}

QString TemplateClassGenerator::identifier() const
{
    return name();
}

void TemplateClassGenerator::setIdentifier(const QString& identifier)
{
    Q_D(TemplateClassGenerator);

    d->renderer.addVariable(QStringLiteral("identifier"), identifier);
    const QStringList separators{
        QStringLiteral("::"),
        QStringLiteral("."),
        QStringLiteral(":"),
        QStringLiteral("\\"),
        QStringLiteral("/"),
    };
    QStringList ns;
    for (const QString& separator : separators) {
        ns = identifier.split(separator);
        if (ns.size() > 1) {
            break;
        }
    }

    setName(ns.takeLast());
    setNamespaces(ns);
}

QStringList TemplateClassGenerator::namespaces() const
{
    Q_D(const TemplateClassGenerator);

    return d->namespaces;
}

void TemplateClassGenerator::setNamespaces(const QStringList& namespaces)
{
    Q_D(TemplateClassGenerator);

    d->namespaces = namespaces;
    d->renderer.addVariable(QStringLiteral("namespaces"), namespaces);
}

/// Specify license for this class
void TemplateClassGenerator::setLicense(const QString& license)
{
    Q_D(TemplateClassGenerator);

    qCDebug(LANGUAGE) << "New Class: " << d->name << "Set license: " << d->license;
    d->license = license;
    d->renderer.addVariable(QStringLiteral("license"), license);
}

/// Get the license specified for this classes
QString TemplateClassGenerator::license() const
{
    Q_D(const TemplateClassGenerator);

    return d->license;
}

void TemplateClassGenerator::setDescription(const ClassDescription& description)
{
    Q_D(TemplateClassGenerator);

    d->description = description;

    QVariantHash variables;
    variables[QStringLiteral("description")] = QVariant::fromValue(description);
    variables[QStringLiteral("members")] = CodeDescription::toVariantList(description.members);
    variables[QStringLiteral("functions")] = CodeDescription::toVariantList(description.methods);
    variables[QStringLiteral("base_classes")] = CodeDescription::toVariantList(description.baseClasses);
    d->renderer.addVariables(variables);
}

ClassDescription TemplateClassGenerator::description() const
{
    Q_D(const TemplateClassGenerator);

    return d->description;
}

void TemplateClassGenerator::addBaseClass(const QString& base)
{
    Q_D(TemplateClassGenerator);

    const InheritanceDescription desc = descriptionFromString(base);

    ClassDescription cd = description();
    cd.baseClasses << desc;
    setDescription(cd);

    DUChainReadLocker lock;

    PersistentSymbolTable::Declarations decl =
        PersistentSymbolTable::self().declarations(IndexedQualifiedIdentifier(QualifiedIdentifier(desc.baseType)));

    //Search for all super classes
    for (PersistentSymbolTable::Declarations::Iterator it = decl.iterator(); it; ++it) {
        DeclarationPointer declaration = DeclarationPointer(it->declaration());
        if (declaration->isForwardDeclaration()) {
            continue;
        }

        // Check if it's a class/struct/etc
        if (declaration->type<StructureType>()) {
            d->fetchSuperClasses(declaration);
            d->directBaseClasses << declaration;
            break;
        }
    }
}

void TemplateClassGenerator::setBaseClasses(const QList<QString>& bases)
{
    Q_D(TemplateClassGenerator);

    // clear
    ClassDescription cd = description();
    cd.baseClasses.clear();
    setDescription(cd);

    d->directBaseClasses.clear();
    d->allBaseClasses.clear();

    // add all bases
    for (const QString& base : bases) {
        addBaseClass(base);
    }
}

QList<DeclarationPointer> TemplateClassGenerator::directBaseClasses() const
{
    Q_D(const TemplateClassGenerator);

    return d->directBaseClasses;
}

QList<DeclarationPointer> TemplateClassGenerator::allBaseClasses() const
{
    Q_D(const TemplateClassGenerator);

    return d->allBaseClasses;
}
