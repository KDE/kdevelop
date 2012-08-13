/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "classgenerator.h"

#include "duchain/persistentsymboltable.h"
#include "duchain/duchainlock.h"
#include "duchain/duchain.h"
#include "duchain/types/structuretype.h"

#include "codegen/documentchangeset.h"
#include "codegen/codedescription.h"
#include <project/projectmodel.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <KLocalizedString>
#include <KDialog>
#include <QPointer>
#include <QVBoxLayout>
#include <QLayout>
#include <QListWidget>
#include <QLabel>

namespace KDevelop {

struct ClassGeneratorPrivate
{
    QString name; ///< The name for the class to be generated (does not include namespace if relevant)
    QString license;
    QList<DeclarationPointer> inheritedClasses;   ///< Represent *ALL* of the inherited classes

    QHash<QString, KUrl> fileUrls;
    QHash<QString, SimpleCursor> filePositions;

    QString headerFileType;
    QString implementationFileType;
    ClassDescription description;
    QStringList namespaces;
};

ClassGenerator::ClassGenerator()
: d(new ClassGeneratorPrivate)
{
    d->headerFileType = "header";
    d->implementationFileType = "implementation";
}

ClassGenerator::~ClassGenerator()
{
    delete d;
}

QString ClassGenerator::name() const
{
    return d->name;
}

void ClassGenerator::setName(const QString& newName)
{
    d->name = newName;
}

QString ClassGenerator::identifier() const
{
    return name();
}

void ClassGenerator::setIdentifier(const QString& identifier)
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

QStringList ClassGenerator::namespaces() const
{
    return d->namespaces;
}

void ClassGenerator::setNamespaces(const QStringList& namespaces)
{
    d->namespaces = namespaces;
}

void ClassGenerator::addDeclaration(const DeclarationPointer& newDeclaration)
{
    m_declarations << newDeclaration;
    if (newDeclaration->isFunctionDeclaration())
    {
        d->description.methods << FunctionDescription(newDeclaration);
    }
    else
    {
        d->description.members << VariableDescription(newDeclaration);
    }
}

QList<DeclarationPointer> ClassGenerator::declarations() const
{
    return m_declarations;
}

QList<DeclarationPointer> ClassGenerator::addBaseClass(const QString& newBaseClass)
{
    QStringList splitBase = newBaseClass.split(' ');
    QString identifier = splitBase.takeLast();
    QString inheritanceMode = splitBase.join(" ");

    InheritanceDescription desc;
    desc.baseType = identifier;
    desc.inheritanceMode = inheritanceMode;
    d->description.baseClasses << desc;

    DUChainReadLocker lock;

    bool added = false;
    PersistentSymbolTable::Declarations decl = PersistentSymbolTable::self().getDeclarations(IndexedQualifiedIdentifier(QualifiedIdentifier(identifier)));

    //Search for all super classes
    for(PersistentSymbolTable::Declarations::Iterator it = decl.iterator(); it; ++it)
    {
        DeclarationPointer declaration = DeclarationPointer(it->declaration());
        if(declaration->isForwardDeclaration())
            continue;

        // Check if it's a class/struct/etc
        if(declaration->type<StructureType>())
        {
            fetchSuperClasses(declaration);
            m_baseClasses << declaration;
            added = true;
            break;
        }
    }

    if(!added)
        m_baseClasses << DeclarationPointer(); //Some entities expect that there is always an item added to the list, so just add zero

    return m_baseClasses;
}

QList<DeclarationPointer> ClassGenerator::inheritanceList() const
{
    return d->inheritedClasses;
}

QList<DeclarationPointer> ClassGenerator::directInheritanceList() const
{
    return m_baseClasses;
}

void ClassGenerator::clearInheritance()
{
    m_baseClasses.clear();
    d->inheritedClasses.clear();
    d->description.baseClasses.clear();
}

void ClassGenerator::clearDeclarations()
{
    m_declarations.clear();
    d->description.members.clear();
    d->description.methods.clear();
}

QHash<QString, QString> ClassGenerator::fileLabels() const
{
    QHash<QString, QString> labels;
    labels.insert(d->headerFileType, i18n("Header"));
    labels.insert(d->implementationFileType, i18n("Implementation"));
    return labels;
}

QHash< QString, KUrl > ClassGenerator::fileUrlsFromBase(const KUrl& baseUrl, bool toLower) const
{
    QHash<QString, KUrl> map;
    map.insert(d->headerFileType, headerUrlFromBase(baseUrl, toLower));
    map.insert(d->implementationFileType, implementationUrlFromBase(baseUrl, toLower));
    return map;
}

KUrl ClassGenerator::headerUrlFromBase(const KUrl& baseUrl, bool toLower) const
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(toLower);
    KUrl url;
    url.addPath(d->name);
    return url;
}

KUrl ClassGenerator::implementationUrlFromBase(const KUrl& baseUrl, bool toLower) const
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(toLower);
    return KUrl();
}

void ClassGenerator::setFilePosition(const QString& fileType, const SimpleCursor& position)
{
    d->filePositions[fileType] = position;
}


void ClassGenerator::setHeaderPosition(const SimpleCursor& position)
{
    setFilePosition(d->headerFileType, position);
}

void ClassGenerator::setImplementationPosition(const SimpleCursor& position)
{
    setFilePosition(d->implementationFileType, position);
}

void ClassGenerator::setFileUrl(const QString& fileType, const KUrl url)
{
    d->fileUrls[fileType] = url;
}

void ClassGenerator::setHeaderUrl(const KUrl& header)
{
    kDebug() << "Header for the generated class: " << header;
    setFileUrl(d->headerFileType, header);
}

void ClassGenerator::setImplementationUrl(const KUrl& implementation)
{
    kDebug() << "Implementation for the generated class: " << implementation;
    setFileUrl(d->implementationFileType, implementation);
}

SimpleCursor ClassGenerator::headerPosition() const
{
    return d->filePositions[d->headerFileType];
}

SimpleCursor ClassGenerator::implementationPosition() const
{
    return d->filePositions[d->implementationFileType];
}

SimpleCursor ClassGenerator::filePosition(const QString& fileType) const
{
    return d->filePositions[fileType];
}

KUrl ClassGenerator::headerUrl() const
{
    return d->fileUrls[d->headerFileType];
}

KUrl ClassGenerator::implementationUrl() const
{
    return d->fileUrls[d->implementationFileType];
}

QHash<QString, KUrl> ClassGenerator::fileUrls() const
{
    return d->fileUrls;
}

void ClassGenerator::setLicense(const QString& license)
{
    kDebug() << "New Class: " << d->name << "Set license: " << d->license;
    d->license = license;
}

QString ClassGenerator::license() const
{
    return d->license;
}

void ClassGenerator::setDescription(const ClassDescription& description)
{
    d->description = description;
}

ClassDescription ClassGenerator::description() const
{
    return d->description;
}

void ClassGenerator::fetchSuperClasses(const DeclarationPointer& derivedClass)
{
    DUChainReadLocker lock;

    //Prevent duplicity
    if(d->inheritedClasses.contains(derivedClass)) {
        return;
    }

    d->inheritedClasses.append(derivedClass);

    DUContext* context = derivedClass->internalContext();
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

void ClassGenerator::addToTarget(const KUrl& url)
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    if (!project)
    {
        return;
    }

    QList<ProjectBaseItem*> items = project->itemsForUrl(url);
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
    if(!target && folder && project->projectFileManager()->features() & KDevelop::IProjectFileManager::Targets )
    {
        /*
         * NOTE: This requires linking agains KDevPlatformProject
         */

        /*
        QList<KDevelop::ProjectTargetItem*> t = folder->targetList();
        for(ProjectBaseItem* it = folder; it && t.isEmpty(); it = it->parent()) {
            t = it->targetList();
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


}
