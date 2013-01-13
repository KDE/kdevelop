/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "simplerefactoring.h"
#include <language/interfaces/codecontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <qaction.h>
#include <klocalizedstring.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <qdialog.h>
#include <qboxlayout.h>
#include <language/duchain/navigation/useswidget.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <QTabWidget>
#include <language/codegen/documentchangeset.h>
#include <qapplication.h>
#include <qdatetime.h>
#include "progressdialogs.h"
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <limits>
#include <language/duchain/use.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <project/projectmodel.h>
#include <language/duchain/functiondefinition.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
//#include "cppnewclass.h"
//#include "makeimplementationprivate.h"
#include <templatedeclaration.h>
#include "../cpputils.h"
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <sourcemanipulation.h>
#include <language/duchain/types/functiontype.h>
#include <kparts/mainwindow.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/classdeclaration.h>
#include <dumptree.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iselectioncontroller.h>

using namespace KDevelop;

// #define WARN_BEFORE_REFACTORING

// static Identifier destructorForName(Identifier name) {
//   QString str = name.identifier().str();
//   if(str.startsWith("~"))
//     return Identifier(str);
//   return Identifier("~"+str);
// }

bool doRefactoringWarning() {
#ifndef WARN_BEFORE_REFACTORING
  return true;
#else
   return KMessageBox::Continue == KMessageBox::warningContinueCancel(0, i18n("Refactoring is an experimental feature, it may damage your code. Before using it, make sure to make a backup."));
#endif
}

KUrl folderFromSelection()
{
    KUrl u;

    KDevelop::Context * sel = ICore::self()->selectionController()->currentSelection();
    KDevelop::FileContext * fc = dynamic_cast<FileContext*>(sel);
    KDevelop::ProjectItemContext * pc = dynamic_cast<ProjectItemContext*>(sel);
    if(fc && !fc->urls().isEmpty())
      u = fc->urls()[0].upUrl();
    else if(pc && !pc->items().isEmpty() && pc->items()[0]->folder())
      ;//TODO check how to solve cyclic dependancy
      //u = pc->items()[0]->folder()->url();
    else if(ICore::self()->documentController()->activeDocument())
      u = ICore::self()->documentController()->activeDocument()->url().upUrl();
    else if(!ICore::self()->projectController()->projects().isEmpty())
      u = ICore::self()->projectController()->projects()[0]->folder();

    return u;
}

SimpleRefactoring& SimpleRefactoring::self() {
  static SimpleRefactoring ret;
  return ret;
}

void SimpleRefactoring::doContextMenu(KDevelop::ContextMenuExtension& extension, KDevelop::Context* context) {

  if(DeclarationContext* declContext = dynamic_cast<DeclarationContext*>(context)){
    //Actions on declarations
    qRegisterMetaType<KDevelop::IndexedDeclaration>("KDevelop::IndexedDeclaration");

    DUChainReadLocker lock(DUChain::lock());

    Declaration* declaration = declContext->declaration().data();

    if(declaration) {
      QFileInfo finfo(declaration->topContext()->url().str());
      if (finfo.isWritable()) {
        QAction* action = new QAction(i18n("Rename %1", declaration->qualifiedIdentifier().toString()), this);
        action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
        action->setIcon(KIcon("edit-rename"));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(executeRenameAction()));

        extension.addAction(ContextMenuExtension::RefactorGroup, action);

        if(declContext->use().isEmpty() && declaration->isFunctionDeclaration() && declaration->internalContext() && declaration->internalContext()->type() == DUContext::Other &&
          !dynamic_cast<Cpp::TemplateDeclaration*>(declaration)) {
          AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(declaration);
          if(funDecl && !funDecl->isInline() && !dynamic_cast<FunctionDefinition*>(funDecl)) {
            //Is a candidate for moving into source
            QAction* action = new QAction(i18n("Create separate definition for %1", declaration->qualifiedIdentifier().toString()), this);
            action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
//           action->setIcon(KIcon("arrow-right"));
            connect(action, SIGNAL(triggered(bool)), this, SLOT(executeMoveIntoSourceAction()));
            extension.addAction(ContextMenuExtension::RefactorGroup, action);
          }
        }
      }
    }
  }
}

QString SimpleRefactoring::moveIntoSource(const IndexedDeclaration& iDecl)
{
  DUChainReadLocker lock;
  Declaration* decl = iDecl.data();
  if(!decl) {
    return i18n("No declaration under cursor");
  }

  KDevelop::IndexedString url = decl->url();
  KUrl targetUrl = decl->url().toUrl();
  if(headerExtensions.contains(QFileInfo(targetUrl.toLocalFile()).suffix())) {
    targetUrl = CppUtils::sourceOrHeaderCandidate(targetUrl);
  }
  if(!targetUrl.isValid()) {
    ///@todo Create source file if it doesn't exist yet
    return i18n("No source file available for %1.", targetUrl.prettyUrl());
  }

  const IndexedString indexedTargetUrl(targetUrl);

  lock.unlock();
  KDevelop::ReferencedTopDUContext top = DUChain::self()->waitForUpdate(url, KDevelop::TopDUContext::AllDeclarationsAndContexts);
  KDevelop::ReferencedTopDUContext targetTopContext = DUChain::self()->waitForUpdate(indexedTargetUrl, KDevelop::TopDUContext::AllDeclarationsAndContexts);
  lock.lock();

  if(!targetTopContext) {
    ///@todo Eventually create source file if it doesn't exist yet
    return i18n("Failed to update DUChain for %1.", targetUrl.prettyUrl());
  }

  if(!top || !iDecl.data() || iDecl.data() != decl) {
    return i18n("Declaration lost while updating.");
  }

  kDebug() << "moving" << decl->qualifiedIdentifier();
  AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl);
  FunctionType::Ptr funType = decl->type<FunctionType>();

  if( !(decl->internalContext() && decl->internalContext()->type() == DUContext::Other
      && funDecl && funDecl->internalFunctionContext() && funType) )
  {
    return i18n("Cannot create definition for this declaration.");
  }

  CodeRepresentation::Ptr code = createCodeRepresentation(decl->url());
  if(!code) {
    return i18n("No document for %1", decl->url().toUrl().prettyUrl());
  }

  SimpleRange headerRange = decl->internalContext()->rangeInCurrentRevision();
  // remove whitespace in front of the header range
  KTextEditor::Range prefixRange(funDecl->internalFunctionContext()->range().end.castToSimpleCursor().textCursor()
                                  + KTextEditor::Cursor(0, 1) /* skip ) of function context */,
                                 headerRange.start.textCursor());
  const QString prefixText = code->rangeText(prefixRange);
  for (int i = prefixText.length() - 1; i >= 0 && prefixText.at(i).isSpace(); --i) {
    if (headerRange.start.column == 0) {
      headerRange.start.line--;
      if (headerRange.start.line == prefixRange.start().line()) {
        headerRange.start.column = prefixRange.start().column() + i;
      } else {
        int lastNewline = prefixText.lastIndexOf('\n', i - 1);
        headerRange.start.column = i - lastNewline - 1;
        kWarning() << "UNSUPPORTED" << headerRange.start.column << lastNewline << i << prefixText;
      }
    } else {
      headerRange.start.column--;
    }
  }
  qDebug() << prefixText << prefixRange;
  const QString body = code->rangeText(headerRange.textRange());
  SourceCodeInsertion ins(targetTopContext);
  QualifiedIdentifier namespaceIdentifier = decl->internalContext()->parentContext()->scopeIdentifier(false);

  ins.setSubScope(namespaceIdentifier);

  QList<SourceCodeInsertion::SignatureItem> signature;

  foreach(Declaration* argument,  funDecl->internalFunctionContext()->localDeclarations()) {
    SourceCodeInsertion::SignatureItem item;
    item.name = argument->identifier().toString();
    item.type = argument->abstractType();
    signature.append(item);
  }

  kDebug() << "qualified id:" << decl->qualifiedIdentifier() << "from mid:" << decl->qualifiedIdentifier().mid(namespaceIdentifier.count()) << namespaceIdentifier.count();

  Identifier id(IndexedString(decl->qualifiedIdentifier().mid(namespaceIdentifier.count()).toString()));
  kDebug() << "id:" << id;

  if(!ins.insertFunctionDeclaration(id, funType->returnType(), signature, funType->modifiers() & AbstractType::ConstModifier, body)) {
    return i18n("Insertion failed");
  }
  lock.unlock();
  DocumentChangeSet::ChangeResult applied = ins.changes().applyAllChanges();
  if(!applied) {
    return i18n("Applying changes failed: %1", applied.m_failureReason);
  }

  // replace header function body with a semicolon
  DocumentChangeSet changeHeader;
  changeHeader.addChange(DocumentChange(decl->url(), headerRange, body, ";"));
  applied = changeHeader.applyAllChanges();
  if(!applied) {
    return i18n("Applying changes failed: %1", applied.m_failureReason);
  }

  ICore::self()->languageController()->backgroundParser()->addDocument(url);
  ICore::self()->languageController()->backgroundParser()->addDocument(indexedTargetUrl);

  return QString();
}

void SimpleRefactoring::executeMoveIntoSourceAction() {
  QAction* action = qobject_cast<QAction*>(sender());
  if(action) {
    IndexedDeclaration iDecl = action->data().value<IndexedDeclaration>();
    if(!iDecl.isValid())
      iDecl = declarationUnderCursor(false);

    const QString error = moveIntoSource(iDecl);
    if (!error.isEmpty()) {
      KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), error);
    }
  }else{
    kWarning() << "strange problem";
  }

}

void SimpleRefactoring::executeRenameAction() {
  QAction* action = qobject_cast<QAction*>(sender());
  if(action) {
    IndexedDeclaration decl = action->data().value<IndexedDeclaration>();
    if(!decl.isValid())
      decl = declarationUnderCursor();
    startInteractiveRename(decl);
  }else{
    kWarning() << "strange problem";
  }
}

class SimpleRefactoringCollector : public KDevelop::UsesWidget::UsesWidgetCollector {
  public:
  SimpleRefactoringCollector(IndexedDeclaration decl) : UsesWidgetCollector(decl) {
    setCollectConstructors(true);
    setCollectDefinitions(true);
    setCollectOverloads(true);
  }

  virtual void processUses(KDevelop::ReferencedTopDUContext topContext) {
    m_allUsingContexts << IndexedTopDUContext(topContext.data());
    UsesWidgetCollector::processUses(topContext);
  }

  QVector<IndexedTopDUContext> m_allUsingContexts;
};

QPair<QString, QString> splitFileAtExtension(const QString& fileName)
{
  int idx = fileName.indexOf('.');
  if (idx == -1) {
    return qMakePair(fileName, QString());
  }
  return qMakePair(fileName.left(idx), fileName.mid(idx));
}

bool SimpleRefactoring::shouldRenameFile(Declaration* declaration)
{
  // only try to rename files when we renamed a class/struct
  if (!dynamic_cast<ClassDeclaration*>(declaration)) {
    return false;
  }
  const KUrl currUrl = declaration->topContext()->url().toUrl();
  const QString fileName = currUrl.fileName();
  const QPair<QString, QString> nameExtensionPair = splitFileAtExtension(fileName);
  // check whether we renamed something that is called like the document it lives in
  return nameExtensionPair.first.compare(declaration->identifier().toString(), Qt::CaseInsensitive) == 0;
}

QString SimpleRefactoring::newFileName(const KUrl& current, const QString& newName)
{
  QPair<QString, QString> nameExtensionPair = splitFileAtExtension(current.fileName());
  // if current file is lowercased, keep that
  if (nameExtensionPair.first == nameExtensionPair.first.toLower()) {
    return newName.toLower() + nameExtensionPair.second;
  } else {
    return newName + nameExtensionPair.second;
  }
}

DocumentChangeSet::ChangeResult SimpleRefactoring::addRenameFileChanges(const KUrl& current,
                                                                        const QString& newName,
                                                                        DocumentChangeSet* changes)
{
  DocumentChangeSet::ChangeResult result = changes->addDocumentRenameChange(
    IndexedString(current), IndexedString(newFileName(current, newName)));
  if (!result) {
    return result;
  }

  // check for implementation file
  const KUrl otherFile = CppUtils::sourceOrHeaderCandidate(current);
  if (otherFile.isValid()) {
    // also rename this other file
    result = changes->addDocumentRenameChange(
        IndexedString(otherFile), IndexedString(newFileName(otherFile, newName)));
    if(!result) {
      return result;
    }
  }
  return true;
}

DocumentChangeSet::ChangeResult applyChangesToDeclarations(const QString& oldName,
                                                           const QString& newName,
                                                           DocumentChangeSet& changes,
                                                           const QList<IndexedDeclaration>& declarations)
{
  foreach(const IndexedDeclaration &decl, declarations) {
    Declaration* declaration = decl.data();
    if(!declaration) {
      continue;
    }
    if (declaration->range().isEmpty()) {
      kDebug() << "found empty declaration";
    }
    TopDUContext* top = declaration->topContext();
    DocumentChangeSet::ChangeResult result = changes.addChange(DocumentChange(top->url(), declaration->rangeInCurrentRevision(), oldName, newName));
    if (!result) {
      return result;
    }

    if (SimpleRefactoring::shouldRenameFile(declaration)) {
      result = SimpleRefactoring::addRenameFileChanges(top->url().toUrl(), newName, &changes);
      if (!result) {
        return result;
      }
    }
  }
  return DocumentChangeSet::ChangeResult(true);
}

DocumentChangeSet::ChangeResult applyChanges(QString oldName, QString newName, DocumentChangeSet& changes, DUContext* context, int usedDeclarationIndex) {
   if(usedDeclarationIndex == std::numeric_limits<int>::max())
     return DocumentChangeSet::ChangeResult(true);

   for(int a = 0; a < context->usesCount(); ++a) {
     const Use& use(context->uses()[a]);
     if(use.m_declarationIndex != usedDeclarationIndex)
       continue;
     if(use.m_range.isEmpty()) {
       kDebug() << "found empty use";
       continue;
     }
     DocumentChangeSet::ChangeResult result = changes.addChange(DocumentChange(context->url(), context->transformFromLocalRevision(use.m_range), oldName, newName));
     if(!result)
       return result;
   }

   foreach(DUContext* child, context->childContexts()) {
     DocumentChangeSet::ChangeResult result = applyChanges(oldName, newName, changes, child, usedDeclarationIndex);
     if(!result)
       return result;
   }
   return DocumentChangeSet::ChangeResult(true);
}

void SimpleRefactoring::startInteractiveRename(KDevelop::IndexedDeclaration decl) {
//   if(!doRefactoringWarning())
//     return;

  DUChainReadLocker lock(DUChain::lock());

  Declaration* declaration = decl.data();
  if(!declaration) {
    KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), i18n("No declaration under cursor"));
    return;
  }
  QFileInfo info(declaration->topContext()->url().str());
  if (!info.isWritable()) {
    KMessageBox::error(ICore::self()->uiController()->activeMainWindow(),
                       i18n("Declaration is located in non-writeable file %1.", declaration->topContext()->url().str()));
    return;
  }

  if(FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(declaration))
  {
    // If this is a function-definition, and there is a separate declaration
    // available, rename that declaration instead
    Declaration* realDeclaration = definition->declaration(declaration->topContext());
    if(realDeclaration)
      declaration = realDeclaration;
  }

  // if renaming a ctor, use the class instead which will trigger renaming of all ctors as well
  if(ClassFunctionDeclaration* cFunc = dynamic_cast<ClassFunctionDeclaration*>(declaration)) {
    if ((cFunc->isConstructor() || cFunc->isDestructor()) && cFunc->context() && cFunc->context()->type() == DUContext::Class && cFunc->context()->owner()) {
      declaration = cFunc->context()->owner();
    }
  }

  if(!declaration)
    return;

  ///Step 1: Allow the user to specify a replacement name, and allow him to see all uses

  QString originalName = declaration->identifier().identifier().str();
  QString replacementName;

  //Since we don't yet know what the text should be replaced with, we just collect the top-contexts to process
  SimpleRefactoringCollector* collector = new SimpleRefactoringCollector(declaration);
  UsesWidget* uses = new UsesWidget(declaration, collector);

  QWidget* navigationWidget = declaration->context()->createNavigationWidget(declaration);
  AbstractNavigationWidget* abstractNavigationWidget = dynamic_cast<AbstractNavigationWidget*>(navigationWidget);

  if(abstractNavigationWidget) { //So the context-links work
    connect(uses, SIGNAL(navigateDeclaration(KDevelop::IndexedDeclaration)),
            abstractNavigationWidget, SLOT(navigateDeclaration(KDevelop::IndexedDeclaration)));
//     connect(uses, SIGNAL(navigateDeclaration(IndexedDeclaration)), tabWidget, SLOT(setCurrentIndex(...)));
///@todo Switch the tab in the tab-widget, so the user will notice that the declaration is being shown
  }

  QScopedPointer<QDialog> dialog(new QDialog);
  dialog->setWindowTitle(i18n("Rename %1", declaration->toString()));

  QVBoxLayout* verticalLayout = new QVBoxLayout;
  dialog->setLayout(verticalLayout);

  QHBoxLayout* actionsLayout = new QHBoxLayout;

  QLabel* newNameLabel = new QLabel(i18n("New name:"));
  actionsLayout->addWidget(newNameLabel);

  QLineEdit* edit = new QLineEdit(declaration->identifier().toString());

  lock.unlock();
  ///NOTE: do not access declaration anymore now!

  newNameLabel->setBuddy(edit);
  edit->setText(originalName);
  edit->setFocus();
  edit->selectAll();
  actionsLayout->addWidget(edit);

  QPushButton* goButton = new QPushButton(i18n("Rename"));
  goButton->setToolTip(i18n("Note: All overloaded functions, overloads, forward-declarations, etc. will be renamed too"));
  actionsLayout->addWidget(goButton);
  connect(goButton, SIGNAL(clicked(bool)), dialog.data(), SLOT(accept()));

  QPushButton* cancelButton = new QPushButton(i18n("Cancel"));
  actionsLayout->addWidget(cancelButton);
  verticalLayout->addLayout(actionsLayout);

  QTabWidget* tabWidget = new QTabWidget;
  verticalLayout->addWidget(tabWidget);
  tabWidget->addTab(uses, i18n("Uses"));
  if (navigationWidget) {
    tabWidget->addTab(navigationWidget, i18n("Declaration Info"));
  }

  connect(cancelButton, SIGNAL(clicked(bool)), dialog.data(), SLOT(reject()));

  dialog->resize( 750, 550 );

  if(dialog->exec() != QDialog::Accepted) {
    kDebug() << "stopped";
    return;
  }
  //It would be nicer to scope this, but then "uses" would not survive

  replacementName = edit->text();


  if(replacementName == originalName || replacementName.isEmpty())
    return;

  //Now just start doing the actual changes, no matter whether the collector is ready or not
  CollectorProgressDialog collectorProgress(i18n("Renaming \"%1\" to \"%2\"", originalName, replacementName), *collector);
  if(!collector->isReady()) {
    collectorProgress.exec();
    if(collectorProgress.result() != QDialog::Accepted) {
      kDebug() << "searching aborted";
      return;
    }
  }

  DocumentChangeSet changes;
  lock.lock();
  foreach(const KDevelop::IndexedTopDUContext &collected, collector->m_allUsingContexts) {
    QSet<int> hadIndices;
    foreach(const IndexedDeclaration &decl, collector->declarations()) {
      uint usedDeclarationIndex = collected.data()->indexForUsedDeclaration(decl.data(), false);
      if(hadIndices.contains(usedDeclarationIndex))
        continue;
      hadIndices.insert(usedDeclarationIndex);
      DocumentChangeSet::ChangeResult result = applyChanges(originalName, replacementName, changes, collected.data(), usedDeclarationIndex);
      if(!result) {
        KMessageBox::error(0, i18n("Applying changes failed: %1", result.m_failureReason));
        return;
      }
    }
  }

  DocumentChangeSet::ChangeResult result = applyChangesToDeclarations(originalName, replacementName, changes, collector->declarations());
  if(!result) {
    KMessageBox::error(0, i18n("Applying changes failed: %1", result.m_failureReason));
    return;
  }

  lock.unlock();
  ///We have to ignore failed changes for now, since uses of a constructor or of operator() may be created on "(" parens
  changes.setReplacementPolicy(DocumentChangeSet::IgnoreFailedChange);
  changes.setFormatPolicy(KDevelop::DocumentChangeSet::NoAutoFormat);

  m_pendingChanges = changes;
  ///NOTE: this is required, otherwise, if you rename a file it will crash...
  QMetaObject::invokeMethod(this, "applyChangesDelayed", Qt::QueuedConnection);
}

void SimpleRefactoring::applyChangesDelayed()
{
  DocumentChangeSet::ChangeResult result = m_pendingChanges.applyAllChanges();
  m_pendingChanges = DocumentChangeSet();
  if(!result) {
      KMessageBox::error(0, i18n("Applying changes failed: %1", result.m_failureReason));
  }
}

KDevelop::IndexedDeclaration SimpleRefactoring::declarationUnderCursor(bool allowUse) {
  KDevelop::IDocument* doc = ICore::self()->documentController()->activeDocument();
  if(doc && doc->textDocument() && doc->textDocument()->activeView()) {
    DUChainReadLocker lock(DUChain::lock());
    if(allowUse)
      return DUChainUtils::itemUnderCursor(doc->url(), SimpleCursor(doc->textDocument()->activeView()->cursorPosition()));
    else
      return DUChainUtils::declarationInLine(SimpleCursor(doc->textDocument()->activeView()->cursorPosition()), DUChainUtils::standardContextForUrl(doc->url()));
  }
  
  return KDevelop::IndexedDeclaration();
}

// #include "simplerefactoring.moc"
