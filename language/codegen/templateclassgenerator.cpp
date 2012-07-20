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
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include "language/codegen/documentchangeset.h"
#include "codedescription.h"
#include "templaterenderer.h"
#include "sourcefiletemplate.h"
#include <duchain/duchainlock.h>
#include <duchain/persistentsymboltable.h>
#include <duchain/types/structuretype.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

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

    QString name;
    QString identifier;
    QStringList namespaces;
    QString license;

    QHash<QString, KUrl> fileUrls;
    QHash<QString, SimpleCursor> filePositions;
    ClassDescription description;

    QList<DeclarationPointer> baseClasses;

    void fetchSuperClasses(const DeclarationPointer& declaration);
};

void TemplateClassGeneratorPrivate::fetchSuperClasses (const DeclarationPointer& declaration)
{
    DUChainReadLocker lock;
    
    //Prevent duplicity
    if(baseClasses.contains(declaration))
    {
        return;
    }

    baseClasses << declaration;
    
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


TemplateClassGenerator::TemplateClassGenerator(const KUrl& baseUrl)
 : d(new TemplateClassGeneratorPrivate)
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
    variables["namespaces"] = namespaces();
    variables["identifier"] = identifier();
    variables["license"] = license();

    ClassDescription desc = description();
    variables["description"] = QVariant::fromValue(desc);
    variables["members"] = CodeDescription::toVariantList(desc.members);
    variables["functions"] = CodeDescription::toVariantList(desc.methods);
    variables["base_classes"] = CodeDescription::toVariantList(desc.baseClasses);

    return variables;
}


DocumentChangeSet TemplateClassGenerator::generate()
{
    Q_ASSERT(d->archive);
    d->renderer.addVariables(templateVariables());

    DocumentChangeSet changes = d->renderer.renderFileTemplate(d->fileTemplate, d->baseUrl, fileUrls());

    addToTarget();
    return changes;
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

QHash< QString, KUrl > TemplateClassGenerator::fileUrls ()
{
    if (d->fileUrls.isEmpty())
    {
        d->renderer.addVariables(templateVariables());
        foreach (const SourceFileTemplate::OutputFile& outputFile, d->fileTemplate->outputFiles())
        {
            KUrl url(d->baseUrl);
            QString outputName = d->renderer.render(outputFile.outputName, outputFile.identifier);
            url.addPath(outputName);
            d->fileUrls.insert(outputFile.identifier, url);
        }
    }

    return d->fileUrls;
}

KUrl TemplateClassGenerator::fileUrl (const QString& outputFile)
{
    return fileUrls().value(outputFile);
}

void TemplateClassGenerator::setFileUrl (const QString& outputFile, const KUrl& url)
{
    d->fileUrls.insert(outputFile, url);
}

SimpleCursor TemplateClassGenerator::filePosition (const QString& outputFile)
{
    return d->filePositions.value(outputFile);
}

void TemplateClassGenerator::setFilePosition (const QString& outputFile, const SimpleCursor& position)
{
    d->filePositions.insert(outputFile, position);
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

TemplateRenderer* TemplateClassGenerator::renderer()
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
}

QString TemplateClassGenerator::identifier() const
{
    return name();
}

void TemplateClassGenerator::setIdentifier(const QString& identifier)
{
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

void TemplateClassGenerator::setNamespaces (const QStringList& namespaces) const
{
    d->namespaces = namespaces;
}


/// Specify license for this class
void TemplateClassGenerator::setLicense(const QString& license)
{
    kDebug() << "New Class: " << d->name << "Set license: " << d->license;
    d->license = license;
}

/// Get the license specified for this classes
QString TemplateClassGenerator::license() const
{
    return d->license;
}


void TemplateClassGenerator::addToTarget()
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(d->baseUrl);
    if (!project)
    {
        return;
    }
    
    QList<ProjectBaseItem*> items = project->itemsForUrl(d->baseUrl);
    if (items.isEmpty())
    {
        return;
    }
    
    ProjectBaseItem* baseItem = items.first();
    
    //Pick the folder Item that should contain the new class
    ProjectFolderItem* folder = baseItem->folder();
    ProjectTargetItem* target = baseItem->target();
    if (target)
    {
        folder = target->parent()->folder();
    }
    else if (!folder)
    {
        folder = baseItem->parent()->folder();
    }
    
    // find target to add created class to
    if(!target && folder && project->projectFileManager()->features() & IProjectFileManager::Targets )
    {
        /*
         * NOTE: This requires linking agains KDevPlatformProject
         */
        
        /*
         *       QList<KDevelop::ProjectTargetItem*> t = folder->targetList();
         *       for(ProjectBaseItem* it = folder; it && t.isEmpty(); it = it->parent()) {
         *           t = it->targetList();
         }
         
         if (t.count() == 1)
         { //Just choose this one
         target = t.first();
    }
    else if (t.count() > 1)
    {
        QPointer<KDialog> dialog = new KDialog;
        QWidget* widget = new QWidget(dialog);
        widget->setLayout(new QVBoxLayout);
        widget->layout()->addWidget(new QLabel(i18n("Choose one target to add the file or cancel if you do not want to do so.")));
        QListWidget* targetsWidget = new QListWidget(widget);
        targetsWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        foreach(ProjectTargetItem* it, t) {
            targetsWidget->addItem(it->text());
    }
    widget->layout()->addWidget(targetsWidget);
    
    targetsWidget->setCurrentRow(0);
    dialog->setButtons( KDialog::Ok | KDialog::Cancel);
    dialog->enableButtonOk(true);
    dialog->setMainWidget(widget);
    
    if(dialog->exec() == QDialog::Accepted)
    {
        if (targetsWidget->selectedItems().isEmpty())
        {
            kDebug() << "Did not select anything, not adding to target";
            // This warning only annoys the user
            // QMessageBox::warning(0, QString(), i18n("Did not select anything, not adding to a target."));
    }
    else
    {
        target = t[targetsWidget->currentRow()];
    }
    }
    }
    */
    }
    
    if (target && project->buildSystemManager())
    {
        QList<ProjectFileItem*> itemsToAdd;
        foreach (const KUrl& url, fileUrls())
        {
            ProjectFileItem* item = project->projectFileManager()->addFile(url, folder);
            if (item)
            {
                itemsToAdd << item;
            }
        }
        project->buildSystemManager()->addFilesToTarget(itemsToAdd, target);
    }
}

void TemplateClassGenerator::setDescription (const ClassDescription& description)
{
    d->description = description;
}

ClassDescription TemplateClassGenerator::description() const
{
    return d->description;
}

void TemplateClassGenerator::addBaseClass (const QString& base)
{
    QStringList splitBase = base.split(' ');
    QString identifier = splitBase.takeLast();
    QString inheritanceMode = splitBase.join(" ");
    
    InheritanceDescription desc;
    desc.baseType = identifier;
    desc.inheritanceMode = inheritanceMode;
    d->description.baseClasses << desc;
    
    DUChainReadLocker lock;
    
    PersistentSymbolTable::Declarations decl = PersistentSymbolTable::self().getDeclarations(IndexedQualifiedIdentifier(QualifiedIdentifier(identifier)));
    
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
            d->baseClasses << declaration;
            break;
        }
    }
}

QList< DeclarationPointer > TemplateClassGenerator::baseClasses()
{
    return d->baseClasses;
}
