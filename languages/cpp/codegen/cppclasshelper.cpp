/*
 * KDevelop C++ Language Support
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "cppclasshelper.h"
#include <codecompletion/missingincludeitem.h>

#include <language/codegen/templateclassassistant.h>
#include <language/codegen/documentchangeset.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include <KLocalizedString>
#include <KTemporaryFile>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

using namespace KDevelop;

CppClassHelper::CppClassHelper (KDevelop::TemplateClassAssistant* assistant) : m_assistant(assistant)
{

}

CppClassHelper::~CppClassHelper()
{

}

TemplateClassGenerator* CppClassHelper::generator()
{
    KUrl url = m_assistant->baseUrl();
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    ProjectBaseItem* item = 0;

    if (project)
    {
        QList<ProjectBaseItem*> items = project->itemsForUrl(url);

        if (!items.isEmpty())
        {
            item = items.first();
        }
    }

    return new CppTemplateNewClass(item);
}

QList< DeclarationPointer > CppClassHelper::defaultMethods(const QString& name)
{
    KTemporaryFile file;
    file.setSuffix( ".cpp" );
    file.setAutoRemove(false);
    file.open();
    QTextStream stream(&file);
    stream << "class " << name << " {\n"
    << "  public:\n"
    // default ctor
    << "    " << name << "();\n"
    // copy ctor
    << "    " << name << "(const " << name << "& other);\n"
    // default dtor
    << "    ~" << name << "();\n"
    // assignment operator
    << "    " << name << "& operator=(const " << name << "& other);\n"
    // equality operator
    << "    bool operator==(const " << name << "& other) const;\n"
    << "};\n";
    file.close();
    ReferencedTopDUContext context(DUChain::self()->waitForUpdate( IndexedString(file.fileName()), KDevelop::TopDUContext::AllDeclarationsAndContexts ));
    DUChainReadLocker lock;

    QList<DeclarationPointer> methods;

    if (context && context->childContexts().size() == 1) {
        foreach (Declaration* declaration, context->childContexts().first()->localDeclarations())
        {
            methods << DeclarationPointer(declaration);
        }
    }

    file.remove();
    return methods;
}

CppTemplateNewClass::CppTemplateNewClass (ProjectBaseItem* parentItem)
: TemplateClassGenerator (parentItem ? parentItem->url() : KUrl())
, m_parentItem(parentItem)
{

}

CppTemplateNewClass::~CppTemplateNewClass()
{

}

QVariantHash CppTemplateNewClass::extraVariables()
{
    QVariantHash variables;

    QMap<QString, VariableDescriptionList> variableDescriptions;

    QMap<QString, FunctionDescriptionList> functionDescriptions;
    QMap<QString, FunctionDescriptionList> slotDescriptions;
    FunctionDescriptionList signalDescriptions;

    foreach (const FunctionDescription& function, description().methods)
    {
        QString access = function.access;
        if (access.isEmpty())
        {
            access = "public";
        }
        if (function.isSignal)
        {
            signalDescriptions << function;
        }
        else if (function.isSlot)
        {
            slotDescriptions[access] << function;
        }
        else
        {
            functionDescriptions[access] << function;
        }
    }

    foreach (const VariableDescription& variable, description().members)
    {
        QString access = variable.access;
        if (access.isEmpty())
        {
            access = "public";
        }
        variableDescriptions[access] << variable;
    }

    QMap<QString, VariableDescriptionList>::const_iterator vit, vend;
    vit = variableDescriptions.constBegin();
    vend = variableDescriptions.constEnd();
    for (; vit != vend; ++vit)
    {
        variables[vit.key() + "_members"] = CodeDescription::toVariantList(vit.value());
    }

    QMap<QString, FunctionDescriptionList>::const_iterator fit, fend;
    fit = functionDescriptions.constBegin();
    fend = functionDescriptions.constEnd();
    for (; fit != fend; ++fit)
    {
        variables[fit.key() + "_functions"] = CodeDescription::toVariantList(fit.value());
    }

    fit = slotDescriptions.constBegin();
    fend = slotDescriptions.constEnd();
    for (; fit != fend; ++fit)
    {
        variables[fit.key() + "_slots"] = CodeDescription::toVariantList(fit.value());
    }

    variables["signals"] = CodeDescription::toVariantList(signalDescriptions);
    variables["needs_qobject_macro"] = !slotDescriptions.isEmpty() || !signalDescriptions.isEmpty();

    QStringList includedFiles;
    DUChainReadLocker locker(DUChain::lock());

    KUrl sourceUrl;
    QHash<QString, KUrl> urls = fileUrls();
    if (!urls.isEmpty())
    {
        sourceUrl = urls.constBegin().value();
    }
    else
    {
        if (m_parentItem)
        {
            sourceUrl = m_parentItem->url();
        }
        // includeDirectiveFromUrl() expects a header URL
        sourceUrl.addPath(name().toLower() + ".h");
    }

    foreach (const DeclarationPointer& base, directBaseClasses())
    {
        if (!base)
        {
            continue;
        }
        kDebug() << "Looking for includes for class" << base->identifier().toString();
        KSharedPtr<Cpp::MissingIncludeCompletionItem> item = Cpp::includeDirectiveFromUrl(sourceUrl, IndexedDeclaration(base.data()));
        if(item)
        {
            kDebug() << "Found one in" << item->m_canonicalPath;
            includedFiles << item->m_addedInclude;
        }
    }
    variables["included_files"] = includedFiles;

    return variables;
}


KDevelop::DocumentChangeSet CppTemplateNewClass::generate()
{
  addVariables(extraVariables());
  KDevelop::DocumentChangeSet changes = TemplateClassGenerator::generate();

  if (!m_parentItem) {
    // happens when we don't select anything in the project view
    // and create a new class from the code menu
    return changes;
  }

  //Pick the folder Item that should contain the new class
  IProject* p=m_parentItem->project();
  ProjectFolderItem* folder=m_parentItem->folder();
  ProjectTargetItem* target=m_parentItem->target();

  if(target) {
    folder=static_cast<ProjectFolderItem*>(m_parentItem->target()->parent());
    Q_ASSERT(folder->folder());
  }else if(!folder) {
    QHash<QString,KUrl> urls = fileUrls();
    Q_ASSERT(!urls.isEmpty());
    if(urls.isEmpty())
      return changes;
    QList<ProjectFolderItem*> folderList = p->foldersForUrl(urls.values().first().upUrl());
    if(folderList.isEmpty())
      return changes;
    folder = folderList.first();
  }

  // find target to add created class to
  if(!target && folder && p->projectFileManager()->features() & KDevelop::IProjectFileManager::Targets ) {
    QList<KDevelop::ProjectTargetItem*> t=folder->targetList();
    for(ProjectBaseItem* it=folder; it && t.isEmpty(); it=it->parent()) {
      t=it->targetList();
    }

    if(t.count()==1) { //Just choose this one
      target=t.first();
    } else if(t.count() > 1) {
      KDialog d;
      QWidget *w=new QWidget(&d);
      w->setLayout(new QVBoxLayout);
      w->layout()->addWidget(new QLabel(i18n("Choose one target to add the file or cancel if you do not want to do so.")));
      QListWidget* targetsWidget=new QListWidget(w);
      targetsWidget->setSelectionMode(QAbstractItemView::SingleSelection);
      foreach(ProjectTargetItem* it, t) {
        targetsWidget->addItem(it->text());
      }
      w->layout()->addWidget(targetsWidget);

      targetsWidget->setCurrentRow(0);
      d.setButtons( KDialog::Ok | KDialog::Cancel);
      d.enableButtonOk(true);
      d.setMainWidget(w);

      if(d.exec()==QDialog::Accepted) {
        if(targetsWidget->selectedItems().isEmpty()) {
          QMessageBox::warning(0, QString(), i18n("Did not select anything, not adding to a target."));
        } else {
          target= t[targetsWidget->currentRow()];
        }
      }
    }
  }

  if(target && p->buildSystemManager()) {
    QList<ProjectFileItem*> itemsToAdd;
    foreach (const KUrl& url, fileUrls())
    {
      if (ProjectFileItem* file = p->projectFileManager()->addFile(url, folder))
      {
        itemsToAdd << file;
      }
    }

    p->buildSystemManager()->addFilesToTarget(itemsToAdd, target);
  }

  return changes;
}

void CppTemplateNewClass::addBaseClass(const QString& base)
{
  //strip access specifier
  QStringList splitBase = base.split(' ', QString::SkipEmptyParts);

  //if no access specifier is found use public by default
  if(splitBase.size() == 1)
  {
    splitBase.prepend("public");
  }

  //Call base function with the access specifier
  TemplateClassGenerator::addBaseClass(splitBase.join(" "));
}

void CppTemplateNewClass::setIdentifier(const QString& identifier)
{
  QStringList list = identifier.split("::");
  setName(list.takeLast());
  setNamespaces(list);
}


