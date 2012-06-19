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
#include "duchain/duchainlock.h"
#include "duchain/duchain.h"
#include "duchain/declaration.h"
#include "duchain/functiondeclaration.h"
#include "duchain/classfunctiondeclaration.h"
#include "duchain/types/functiontype.h"
#include "duchain/types/integraltype.h"
#include "duchain/types/referencetype.h"
#include "duchain/types/pointertype.h"

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

GRANTLEE_BEGIN_LOOKUP(DeclarationPointer)
    if (!object.data())
    {
        return QVariant();
    }
    if ( property == "identifier" )
        return object->identifier().toString();
    else if ( property == "toString" )
        return object->toString();
    else if ( property == "type" )
    {
        AbstractType::Ptr type = object->abstractType();
        FunctionType::Ptr functionType = type.cast<FunctionType>();
        if (functionType)
        {
            type = functionType->returnType();
        }
        return type ? type->toString() : QString();
    }
    else if ( property == "internal_declarations" )
    {
        QList<DeclarationPointer> list;
        foreach (Declaration* declaration, object->internalContext()->localDeclarations())
        {
            list << DeclarationPointer(declaration);
        }
        return QVariant::fromValue(list);
    }
    else if ( property == "default_return_value" )
    {
        FunctionType::Ptr functionType = object->abstractType().cast<FunctionType>();
        if (functionType)
        {
            AbstractType::Ptr type = functionType->returnType();
            if (IntegralType::Ptr integralType = type.cast<IntegralType>())
            {
                return QVariant(integralType->toString() + "()");
            }
            else if (ReferenceType::Ptr refType = type.cast<ReferenceType>())
            {
                return QVariant(refType->baseType()->toString() + "()");
            }
            else if (PointerType::Ptr pointerType = type.cast<PointerType>())
            {
                return QString("0");
            }
        }
    }
    else if ( property == "is_virtual" )
    {
        if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(object.data()))
        {
            return function->isVirtual();
        }
        return false;
    }
    else if ( property == "is_static" )
    {
        if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(object.data()))
        {
            return function->isStatic();
        }
        return false;
    }
    else if ( property == "is_constructor" )
    {
        if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(object.data()))
        {
            return function->isConstructor();
        }
        return false;
    }
    else if ( property == "is_destructor" )
    {
        if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(object.data()))
        {
            return function->isDestructor();
        }
        return false;
    }
GRANTLEE_END_LOOKUP

class NoEscapeStream : public Grantlee::OutputStream
{
public:
    NoEscapeStream();
    explicit NoEscapeStream (QTextStream* stream);

    virtual QString escape (const QString& input) const;
    virtual QSharedPointer< OutputStream > clone (QTextStream* stream) const;
};

NoEscapeStream::NoEscapeStream() : OutputStream()
{

}

NoEscapeStream::NoEscapeStream (QTextStream* stream) : OutputStream (stream)
{

}

QString NoEscapeStream::escape (const QString& input) const
{
    return input;
}

QSharedPointer< Grantlee::OutputStream > NoEscapeStream::clone (QTextStream* stream) const
{
    QSharedPointer<OutputStream> clonedStream = QSharedPointer<OutputStream>( new NoEscapeStream( stream ) );
    return clonedStream;
}

class KDevelop::TemplateClassGeneratorPrivate
{
public:
    QString templateDescription;
    KArchive* archive;
    KUrl baseUrl;
    ArchiveTemplateLoader* loader;
    QVariantHash variables;

    void loadTemplate();
    QString render(Grantlee::Template t, Grantlee::Context& c);
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
}

QString TemplateClassGeneratorPrivate::render (Grantlee::Template t, Grantlee::Context& c)
{
    QString ret;
    QTextStream textStream(&ret);
    NoEscapeStream stream(&textStream);

    t->render(&stream, &c);
    return ret;
}


TemplateClassGenerator::TemplateClassGenerator(const KUrl& baseUrl) : ClassGenerator(),
d(new TemplateClassGeneratorPrivate)
{
    d->archive = 0;
    d->baseUrl = baseUrl;

    Grantlee::registerMetaType<DeclarationPointer>();
    Grantlee::registerMetaType<VariableDescription>();
    Grantlee::registerMetaType<FunctionDescription>();
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
    QVariantHash variables = d->variables;

    variables["name"] = name();
    variables["identifier"] = identifier();
    variables["license"] = license();

    variables["inheritance_list"] = QVariant::fromValue(inheritanceList());
    variables["is_inherited"] = !directInheritanceList().isEmpty();
    variables["direct_inheritance_list"] = QVariant::fromValue(directInheritanceList());
    variables["declarations"] = QVariant::fromValue(declarations());

    ClassDescription desc = description();
    variables["properties"] = QVariant::fromValue(desc.members);
    variables["methods"] = QVariant::fromValue(desc.methods);

    kDebug() << "Class description:" << desc.members.size() << desc.methods.size();

    QList<DeclarationPointer> functions;
    QList<DeclarationPointer> members;

    foreach (const DeclarationPointer& pointer, declarations())
    {
        if (pointer->isFunctionDeclaration())
        {
            functions << pointer;
        }
        else
        {
            members << pointer;
        }
    }

    variables["functions"] = QVariant::fromValue(functions);
    variables["members"] = QVariant::fromValue(members);

    kDebug() << "Inheritance sizes:" << directInheritanceList().size() << inheritanceList().size();

    QList<DeclarationPointer> methods;
    QList<DeclarationPointer> properties;

    DUChainReadLocker lock(DUChain::lock());
    foreach (const DeclarationPointer& pointer, declarations())
    {
        Declaration* decl = pointer.data();
        if (!decl)
        {
            continue;
        }

        if (decl->isFunctionDeclaration())
        {
            methods << pointer;
        }
        else
        {
            properties << pointer;
        }
    }

    variables["method_declarations"] = QVariant::fromValue(methods);
    variables["property_declarations"] = QVariant::fromValue(properties);

    d->baseUrl.adjustPath(KUrl::AddTrailingSlash);
    QHash<QString,KUrl> urls = fileUrls();
    for (QHash<QString,KUrl>::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        variables["output_file_" + it.key().toLower()] = KUrl::relativeUrl(d->baseUrl, it.value());
        variables["output_file_" + it.key().toLower() + "_absolute"] = it.value().toLocalFile();
    }

    return variables;
}


DocumentChangeSet TemplateClassGenerator::generate()
{
    Q_ASSERT(d->archive);
    QVariantHash variables = templateVariables();

    DocumentChangeSet changes;

    Grantlee::Engine engine;
    engine.setSmartTrimEnabled(true);

    Grantlee::FileSystemTemplateLoader* loader = new Grantlee::FileSystemTemplateLoader;
    loader->setTemplateDirs(ICore::self()->componentData().dirs()->findDirs("data", "kdevcodegen/templates"));
    engine.addTemplateLoader(Grantlee::AbstractTemplateLoader::Ptr(loader));

    foreach (const QString& path, ICore::self()->componentData().dirs()->resourceDirs("lib"))
    {
        engine.addPluginPath(path);
    }

    Grantlee::Context context(variables);

    // TODO: Add more variables to context

    const KArchiveDirectory* dir = d->archive->directory();

    d->loader = new ArchiveTemplateLoader(dir);
    engine.addTemplateLoader(Grantlee::AbstractTemplateLoader::Ptr(d->loader));



    kDebug() << "Opened archive with contents:" << dir->entries();

    DUChainReadLocker lock(DUChain::lock());

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

        Grantlee::Template fileTemplate = engine.newTemplate(file->data(), it.key());

        DocumentChange change(document, range, QString(), d->render(fileTemplate, context));
        if (fileTemplate->error() != Grantlee::NoError)
        {
            kDebug() << fileTemplate->errorString();
        }
        changes.addChange(change);
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

    Grantlee::Engine engine;

    Grantlee::Context context(templateVariables());

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

        Grantlee::Template nameTemplate = engine.newTemplate(cg.readEntry("OutputFile"), cg.name());

        KUrl url(baseUrl);
        QString outputName = nameTemplate->render(&context);
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
    for (QVariantHash::const_iterator it = variables.constBegin(); it != variables.constEnd(); ++it)
    {
        d->variables.insert(it.key(), it.value());
    }
}

QString TemplateClassGenerator::renderString(const QString& text)
{
    Grantlee::Engine engine;

    QVariantHash variables = templateVariables();
    kDebug() << variables;

    Grantlee::Context context(variables);

    Grantlee::Template t = engine.newTemplate(text, QString());
    return t->render(&context);
}