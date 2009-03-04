/*
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

#include "codeassistant.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <interfaces/iuicontroller.h>
#include <ktexteditor/view.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/declaration.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/functiondefinition.h>
#include <klocalizedstring.h>
#include <language/codegen/documentchangeset.h>
#include <kmessagebox.h>

using namespace KDevelop;

namespace Cpp {

StaticCodeAssistant staticCodeAssistant;

StaticCodeAssistant::StaticCodeAssistant() {
  connect(KDevelop::ICore::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)), SLOT(documentLoaded(KDevelop::IDocument*)));
  foreach(KDevelop::IDocument* document, KDevelop::ICore::self()->documentController()->openDocuments())
    documentLoaded(document);
}

void StaticCodeAssistant::documentLoaded(KDevelop::IDocument* document) {
  ///@todo Also wait for "textRemoved" signal
  if(document->textDocument())
    connect(document->textDocument(), SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
}

QString CreateDeclarationAction::description() const {
  return "create declaration";
}

void CreateDeclarationAction::execute() {
  kDebug() << "executing action";
}

bool AdaptDefinitionSignatureAssistant::isUseful() {
  kDebug() << m_declarationName.toString() << m_definitionId.qualifiedIdentifier().toString();
  return !m_declarationName.isEmpty() && m_definitionId.isValid();
}

DUContext* getFunctionContext(Declaration* decl) {
  DUContext* functionContext = decl->internalContext();
  if(functionContext && functionContext->type() != DUContext::Function) {
    foreach(DUContext::Import import, functionContext->importedParentContexts()) {
      DUContext* ctx = import.context(decl->topContext());
      if(ctx && ctx->type() == DUContext::Function)
        functionContext = ctx;
    }
  }
  
  if(functionContext && functionContext->type() == DUContext::Function)
    return functionContext;
  return 0;
}

AdaptDefinitionSignatureAssistant::AdaptDefinitionSignatureAssistant(KTextEditor::View* view, KTextEditor::Range inserted) : ITextAssistant(view) {
  connect(KDevelop::ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), SLOT(parseJobFinished(KDevelop::ParseJob*)));
  m_document = KDevelop::IndexedString(view->document()->url());
  
  kDebug() << "checking";
  
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 300);
  if(!lock.locked()) {
    kDebug() << "failed to lock duchain in time";
    return;
  }
  
  DUContext* context = findFunctionContext(m_document.toUrl(), SimpleCursor(inserted.start()));
  
  if(!context) {
    kDebug() << "not found function-context";
    return;
  }
  
  Declaration* funDecl = context->owner();
  m_declarationName = funDecl->identifier();
  kDebug() << "found local function declaration:" << m_declarationName.toString();
  m_document = context->url();
  
  KDevelop::FunctionDefinition* definition = KDevelop::FunctionDefinition::definition(funDecl);
  if(definition) {
    //The declaration has a separate definition, so it can be adapted.
    m_definitionId = definition->id();
    kDebug() << "found definition" << m_definitionId.qualifiedIdentifier().toString();
    m_definitionContext = KDevelop::ReferencedTopDUContext(definition->topContext());
    
    DUContext* definitionFunctionContext = getFunctionContext(definition);
    
    if(!definitionFunctionContext) {
      kDebug() << "no function-context for definition";
      return;
    }
    
    foreach(Declaration* parameter, definitionFunctionContext->localDeclarations())
      m_oldSignature << qMakePair(parameter->indexedType(), parameter->identifier().identifier());
    
    //Schedule an update, to make sure the ranges match
    DUChain::self()->updateContextForUrl(m_definitionContext->url(), KDevelop::TopDUContext::AllDeclarationsAndContexts);
  }else{
    kDebug() << "not found definition";
  }
}

DUContext* AdaptDefinitionSignatureAssistant::findFunctionContext(KUrl url, KDevelop::SimpleCursor position) const {
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  TopDUContext* top = DUChainUtils::standardContextForUrl(url);
  if(top) {
    DUContext* context = top->findContextAt(position, true);
    if(context && context->type() == DUContext::Function && context->owner())
      return context;
  }
  return 0;
}

QString makeSignatureString(QList<SignatureItem> signature) {
  QString ret;
  foreach(SignatureItem item, signature) {
    if(!ret.isEmpty())
      ret += ", ";
    ret += (item.first.abstractType() ? item.first.abstractType()->toString() : QString("<none>"));
    
    if(!item.second.str().isEmpty())
      ret += " " + item.second.str();
  }
  return ret;
}

class AdaptSignatureAction : public KDevelop::IAssistantAction {
  public:
    AdaptSignatureAction(KDevelop::DeclarationId definitionId, KDevelop::ReferencedTopDUContext definitionContext, QList<SignatureItem> oldSignature, QList<SignatureItem> newSignature) : 
    m_definitionId(definitionId), 
    m_definitionContext(definitionContext), 
    m_oldSignature(oldSignature),
    m_newSignature(newSignature) {
    }
    
    virtual QString description() const {
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
      return i18n("Adapt definition signature of %1(%2) to to (%3)", m_definitionId.qualifiedIdentifier().toString(), makeSignatureString(m_oldSignature), makeSignatureString(m_newSignature));
    }
    
    virtual void execute() {
      
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
      IndexedString url = m_definitionContext->url();
      lock.unlock();
      m_definitionContext = DUChain::self()->waitForUpdate(url, TopDUContext::AllDeclarationsContextsAndUses);
      if(!m_definitionContext) {
        kDebug() << "failed to update" << url.str();
        return;
      }

      lock.lock();
      
      Declaration* definition = m_definitionId.getDeclaration(m_definitionContext.data());
      if(!definition) {
        kDebug() << "could not find definition";
        return;
      }
      
      DUContext* functionContext = getFunctionContext(definition);
      if(!functionContext) {
        kDebug() << "no function context";
        return;
      }
      
      ///@todo Do matching of the arguments
      ///@todo Eventually do renaming
      if(!functionContext || functionContext->type() != DUContext::Function) {
        kDebug() << "no correct function context";
        return;
      }
      
      DocumentChangeSet changes;
      DocumentChangePointer change(new DocumentChange(functionContext->url(), functionContext->range(), QString(), makeSignatureString(m_newSignature)));
      change->m_ignoreOldText = true;
      changes.addChange( change );
      DocumentChangeSet::ChangeResult result = changes.applyAllChanges(DocumentChangeSet::WarnOnFailedChange);
      if(!result) {
        KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
      }
    }
    
    KDevelop::DeclarationId m_definitionId;
    KDevelop::ReferencedTopDUContext m_definitionContext;
    QList<SignatureItem> m_oldSignature;
    QList<SignatureItem> m_newSignature;
};

void AdaptDefinitionSignatureAssistant::parseJobFinished(KDevelop::ParseJob* job) {
  if(job->document() == m_document) {
    kDebug() << "parse job finshed for current document";
    clearActions();
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(DUContext* context = findFunctionContext(m_document.toUrl(), SimpleCursor(invocationCursor()))) {
      if(context->owner() && context->owner()->identifier() == m_declarationName) {
        QList<SignatureItem> newSignature;
        foreach(Declaration* parameter, context->localDeclarations()) {
          kDebug() << "parameter:" << parameter->toString() << parameter->range().textRange();
          newSignature << qMakePair(parameter->indexedType(), parameter->identifier().identifier());
        }

        if(newSignature != m_oldSignature) {
          kDebug() << "signature changed";
          addAction(IAssistantAction::Ptr(new AdaptSignatureAction(m_definitionId, m_definitionContext, m_oldSignature, newSignature)));
        }else{
          kDebug() << "signature stayed equal";
        }
      }
    }
    
    emit actionsChanged();
  }
}

void StaticCodeAssistant::assistantHide() {
  m_activeAssistant.clear();
}

void StaticCodeAssistant::textInserted(KTextEditor::Document* document, KTextEditor::Range range) {
  QString inserted = document->text(range);
  
  if(m_activeAssistant) {
    kDebug() << "there still is an active assistant";
    return;
  }
  //Eventually pop up an assistant
  if(!document->activeView())
    return;
  
  KSharedPtr<AdaptDefinitionSignatureAssistant> signatureAssistant(new AdaptDefinitionSignatureAssistant(document->activeView(), range));
  
  if(signatureAssistant->isUseful()) {
    m_activeAssistant = KSharedPtr<IAssistant>(signatureAssistant.data());
    connect(m_activeAssistant.data(), SIGNAL(hide()), SLOT(assistantHide()));
    ICore::self()->uiController()->popUpAssistant(m_activeAssistant);
  }

}

}

#include "codeassistant.moc"
