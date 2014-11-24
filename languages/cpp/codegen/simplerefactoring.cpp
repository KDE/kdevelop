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

#include "../cpputils.h"
#include "../debug.h"

#include <QAction>
#include <KMessageBox>
#include <ktexteditor/document.h>
#include <kparts/mainwindow.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/use.h>
#include <language/interfaces/codecontext.h>

#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>

#include <templatedeclaration.h>
#include <sourcemanipulation.h>

using namespace KDevelop;



SimpleRefactoring::SimpleRefactoring(QObject *parent)
  : BasicRefactoring(parent)
{
  /* There's nothing to do here. */
}

void SimpleRefactoring::fillContextMenu(KDevelop::ContextMenuExtension& extension, KDevelop::Context* context) {

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
        action->setIcon(QIcon::fromTheme("edit-rename"));
        connect(action, &QAction::triggered, this, &SimpleRefactoring::executeRenameAction);

        extension.addAction(ContextMenuExtension::RefactorGroup, action);

        if(declContext->use().isEmpty() && declaration->isFunctionDeclaration() && declaration->internalContext() && declaration->internalContext()->type() == DUContext::Other &&
          !dynamic_cast<Cpp::TemplateDeclaration*>(declaration)) {
          AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(declaration);
          if(funDecl && !funDecl->isInline() && !dynamic_cast<FunctionDefinition*>(funDecl)) {
            //Is a candidate for moving into source
            QAction* action = new QAction(i18n("Create separate definition for %1", declaration->qualifiedIdentifier().toString()), this);
            action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
//           action->setIcon(QIcon::fromTheme("arrow-right"));
            connect(action, &QAction::triggered, this, &SimpleRefactoring::executeMoveIntoSourceAction);
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

  const KDevelop::IndexedString url = decl->url();
  QString targetUrl = url.str();
  if(CppUtils::headerExtensions().contains(QFileInfo(targetUrl).suffix())) {
    targetUrl = CppUtils::sourceOrHeaderCandidate(targetUrl);
  }
  if(targetUrl.isEmpty()) {
    ///@todo Create source file if it doesn't exist yet
    return i18n("No source file available for %1.", url.str());
  }

  lock.unlock();

  const IndexedString indexedTargetUrl(targetUrl);
  KDevelop::ReferencedTopDUContext top = DUChain::self()->waitForUpdate(url, KDevelop::TopDUContext::AllDeclarationsAndContexts);
  KDevelop::ReferencedTopDUContext targetTopContext = DUChain::self()->waitForUpdate(indexedTargetUrl, KDevelop::TopDUContext::AllDeclarationsAndContexts);
  lock.lock();

  if(!targetTopContext) {
    ///@todo Eventually create source file if it doesn't exist yet
    return i18n("Failed to update DUChain for %1.", targetUrl);
  }

  if(!top || !iDecl.data() || iDecl.data() != decl) {
    return i18n("Declaration lost while updating.");
  }

  qCDebug(CPP) << "moving" << decl->qualifiedIdentifier();
  AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl);
  FunctionType::Ptr funType = decl->type<FunctionType>();

  if( !(decl->internalContext() && decl->internalContext()->type() == DUContext::Other
      && funDecl && funDecl->internalFunctionContext() && funType) )
  {
    return i18n("Cannot create definition for this declaration.");
  }

  CodeRepresentation::Ptr code = createCodeRepresentation(decl->url());
  if(!code) {
    return i18n("No document for %1", decl->url().str());
  }

  KTextEditor::Range headerRange = decl->internalContext()->rangeInCurrentRevision();
  // remove whitespace in front of the header range
  KTextEditor::Range prefixRange(funDecl->internalFunctionContext()->range().end.castToSimpleCursor()
                                  + KTextEditor::Cursor(0, 1) /* skip ) of function context */,
                                 headerRange.start());
  const QString prefixText = code->rangeText(prefixRange);
  for (int i = prefixText.length() - 1; i >= 0 && prefixText.at(i).isSpace(); --i) {
    if (headerRange.start().column() == 0) {
      headerRange.start().setLine(headerRange.start().line() - 1);
      if (headerRange.start().line() == prefixRange.start().line()) {
        headerRange.start().setColumn(prefixRange.start().column() + i);
      } else {
        int lastNewline = prefixText.lastIndexOf('\n', i - 1);
        headerRange.start().setColumn(i - lastNewline - 1);
        qCWarning(CPP) << "UNSUPPORTED" << headerRange.start().column() << lastNewline << i << prefixText;
      }
    } else {
      headerRange.start().setColumn(headerRange.start().column() - 1);
    }
  }
  const QString body = code->rangeText(headerRange);
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

  qCDebug(CPP) << "qualified id:" << decl->qualifiedIdentifier() << "from mid:" << decl->qualifiedIdentifier().mid(namespaceIdentifier.count()) << namespaceIdentifier.count();

  Identifier id(IndexedString(decl->qualifiedIdentifier().mid(namespaceIdentifier.count()).toString()));
  qCDebug(CPP) << "id:" << id;

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
    qCWarning(CPP) << "strange problem";
  }

}

DocumentChangeSet::ChangeResult SimpleRefactoring::addRenameFileChanges(const QUrl &current, const QString& newName, DocumentChangeSet* changes)
{
  auto result = KDevelop::BasicRefactoring::addRenameFileChanges(current, newName, changes);
  if (!result) {
    return result;
  }

  // check for implementation file
  const QUrl otherFile = QUrl::fromLocalFile(CppUtils::sourceOrHeaderCandidate(current.toLocalFile()));
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

void SimpleRefactoring::startInteractiveRename(const KDevelop::IndexedDeclaration &decl)
{
    QString originalName;
    Declaration* declaration = nullptr;
    {
        DUChainReadLocker lock;

        declaration = decl.data();
        if (!declaration) {
            KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), i18n("No declaration under cursor"));
            return;
        }

        QFileInfo info(declaration->topContext()->url().str());
        if (!info.isWritable()) {
            KMessageBox::error(ICore::self()->uiController()->activeMainWindow(),
                               i18n("Declaration is located in non-writeable file %1.", declaration->topContext()->url().str()));
            return;
        }

        if (FunctionDefinition* definition = dynamic_cast<FunctionDefinition*>(declaration)) {
            // If this is a function-definition, and there is a separate declaration
            // available, rename that declaration instead
            Declaration* realDeclaration = definition->declaration(declaration->topContext());
            if (realDeclaration)
                declaration = realDeclaration;
        }

        // if renaming a ctor, use the class instead which will trigger renaming of all ctors as well
        if (ClassFunctionDeclaration* cFunc = dynamic_cast<ClassFunctionDeclaration*>(declaration)) {
            if ((cFunc->isConstructor() || cFunc->isDestructor()) && cFunc->context() && cFunc->context()->type() == DUContext::Class && cFunc->context()->owner()) {
                declaration = cFunc->context()->owner();
            }
        }

        if (!declaration)
            return;

        originalName = declaration->identifier().identifier().str();
    }

    NameAndCollector nc = newNameForDeclaration(DeclarationPointer(declaration));
    if (nc.newName == originalName || nc.newName.isEmpty())
        return;

    DocumentChangeSet changes = BasicRefactoring::renameCollectedDeclarations(nc.collector.data(), nc.newName, originalName, false);

    changes.setFormatPolicy(KDevelop::DocumentChangeSet::NoAutoFormat);

    m_pendingChanges = changes;
    ///NOTE: this is required, otherwise, if you rename a file it will crash...
    QMetaObject::invokeMethod(this, "applyChangesDelayed", Qt::QueuedConnection);
}

DocumentChangeSet::ChangeResult SimpleRefactoring::applyChangesToDeclarations(
        const QString& oldName,
        const QString& newName,
        DocumentChangeSet& changes,
        const QList<IndexedDeclaration>& declarations)
{
    auto result = BasicRefactoring::applyChangesToDeclarations(oldName, newName, changes, declarations);
    if (!result) {
        return result;
    }

    for (const auto & decl : declarations) {
        Declaration* declaration = decl.data();
        if (!declaration) {
            continue;
        }
        if (shouldRenameFile(declaration)) {
            result = addRenameFileChanges(declaration->topContext()->url().toUrl(), newName, &changes);
            if (!result) {
                return result;
            }
        }
    }
    return DocumentChangeSet::ChangeResult(true);
}

void SimpleRefactoring::applyChangesDelayed()
{
  DocumentChangeSet::ChangeResult result = m_pendingChanges.applyAllChanges();
  m_pendingChanges = DocumentChangeSet();
  if(!result) {
      KMessageBox::error(0, i18n("Applying changes failed: %1", result.m_failureReason));
  }
}

// #include "simplerefactoring.moc"
