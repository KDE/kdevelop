/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#include "signatureassistant.h"
#include <language/duchain/duchainutils.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/declaration.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/functiondefinition.h>
#include <klocalizedstring.h>
#include <language/codegen/documentchangeset.h>
#include <kmessagebox.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/parsingenvironment.h>
#include "signatureassistant.h"
#include "cppduchain.h"

using namespace  KDevelop;
using namespace Cpp;

bool AdaptDefinitionSignatureAssistant::isUseful() {
  kDebug() << m_declarationName.toString() << m_definitionId.qualifiedIdentifier().toString();
  return !m_declarationName.isEmpty() && m_definitionId.isValid();
}

AdaptDefinitionSignatureAssistant::AdaptDefinitionSignatureAssistant(KTextEditor::View* view, KTextEditor::Range inserted) : ITextAssistant(view) {
  connect(KDevelop::ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), SLOT(parseJobFinished(KDevelop::ParseJob*)));
  m_document = KDevelop::IndexedString(view->document()->url());
  
  m_invocationRange = SimpleRange(inserted);
  
  kDebug() << "checking";
  
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 300);
  if(!lock.locked()) {
    kDebug() << "failed to lock duchain in time";
    return;
  }
  TopDUContext* top = DUChainUtils::standardContextForUrl(m_document.toUrl());
  if(!top)
    return;
  
  Declaration* funDecl = DUChainUtils::declarationInLine(m_invocationRange.start, top);
  
  if(!funDecl || !funDecl->type<KDevelop::FunctionType>()) {
    kDebug() << "no declaration found in line";
    return;
  }
  
  m_declarationName = funDecl->identifier();
  kDebug() << "found local function declaration:" << m_declarationName.toString();
  m_document = top->url();
  
  KDevelop::FunctionDefinition* definition = dynamic_cast<KDevelop::FunctionDefinition*>(funDecl);
  
  KDevelop::Declaration* otherSide = 0;
  
  if(definition) {
    m_editingDefinition = true;
    otherSide = definition->declaration(top);
    if(otherSide) {
      kDebug() << "found declaration" << otherSide->qualifiedIdentifier().toString();
      
      m_definitionId = otherSide->id();
      m_definitionContext = KDevelop::ReferencedTopDUContext(otherSide->topContext());
    }
  }else if((definition = KDevelop::FunctionDefinition::definition(funDecl))) {
    m_editingDefinition = false;
    
    otherSide = definition;
    
    kDebug() << "found definition" << m_definitionId.qualifiedIdentifier().toString();
    
    m_definitionId = definition->id();
    m_definitionContext = KDevelop::ReferencedTopDUContext(definition->topContext());
  }
  
  if(!otherSide) {
    kDebug() << "not found other side";
    return;
  }
  
  DUContext* otherSideFunctionContext = DUChainUtils::getFunctionContext(otherSide);
  AbstractFunctionDeclaration* otherFunDecl = dynamic_cast<AbstractFunctionDeclaration*>(otherSide);
  
  if(!otherSideFunctionContext || !otherFunDecl) {
    kDebug() << "no function-context for definition";
    return;
  }
  
  int pos = 0;
  foreach(Declaration* parameter, otherSideFunctionContext->localDeclarations()) {
    QString id = parameter->identifier().identifier().str();
    QString defaultParam = otherFunDecl->defaultParameterForArgument(pos).str();
    if(!defaultParam.isEmpty())
      id += " = " + defaultParam;
    
    m_oldSignature << qMakePair(parameter->indexedType(), id);
    ++pos;
  }
  
  KDevelop::FunctionType::Ptr funType = otherSide->type<KDevelop::FunctionType>();
  if(funType)
    m_oldReturnType = funType->returnType()->indexed();
  
  //Schedule an update, to make sure the ranges match
  DUChain::self()->updateContextForUrl(m_definitionContext->url(), KDevelop::TopDUContext::AllDeclarationsAndContexts);
  
}

DUContext* AdaptDefinitionSignatureAssistant::findFunctionContext(KUrl url, KDevelop::SimpleRange range) const {
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  TopDUContext* top = DUChainUtils::standardContextForUrl(url);

  if(top) {
    DUContext* context = top->findContextAt(range.start, true);
    if(context == top)
      context = top->findContextAt(range.end, true);
    
    if(context && context->type() == DUContext::Function && context->owner()) {
      return context;
    }
  }
  return 0;
}

QString makeSignatureString(QList<SignatureItem> signature, DUContext* visibilityFrom) {
  QString ret;
  foreach(SignatureItem item, signature) {
    if(!ret.isEmpty())
      ret += ", ";
    ret += Cpp::simplifiedTypeString(item.first.abstractType(),  visibilityFrom);
    
    if(!item.second.isEmpty())
      ret += " " + item.second;
  }
  return ret;
}
int min(int a, int b) {
  return a < b ? a : b;
}

class AdaptSignatureAction : public KDevelop::IAssistantAction {
  public:
    AdaptSignatureAction(KDevelop::DeclarationId definitionId, KDevelop::ReferencedTopDUContext definitionContext, QList<SignatureItem> oldSignature, QList<SignatureItem> newSignature) : 
    m_otherSideId(definitionId), 
    m_otherSideContext(definitionContext), 
    m_oldSignature(oldSignature),
    m_newSignature(newSignature) {
    }
    
    virtual QString description() const {
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
      return i18n("Update Definition from %1(%2) to (%3)", m_otherSideId.qualifiedIdentifier().toString(), makeSignatureString(m_oldSignature, m_otherSideContext.data()), makeSignatureString(m_newSignature, m_otherSideContext.data()));
    }
    
    virtual void execute() {
      
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
      IndexedString url = m_otherSideContext->url();
      lock.unlock();
      m_otherSideContext = DUChain::self()->waitForUpdate(url, TopDUContext::AllDeclarationsContextsAndUses);
      if(!m_otherSideContext) {
        kDebug() << "failed to update" << url.str();
        return;
      }

      lock.lock();
      
      Declaration* otherSide = m_otherSideId.getDeclaration(m_otherSideContext.data());
      if(!otherSide) {
        kDebug() << "could not find definition";
        return;
      }
      
      DUContext* functionContext = DUChainUtils::getFunctionContext(otherSide);
      if(!functionContext) {
        kDebug() << "no function context";
        return;
      }

      ///@todo Handle return-type
      ///@todo Keep default-parameters
      ///@todo Do matching of the arguments
      ///@todo Eventually do real renaming
      if(!functionContext || functionContext->type() != DUContext::Function) {
        kDebug() << "no correct function context";
        return;
      }
      
      for(int a = 0; a < min(m_oldSignature.size(), m_newSignature.size()); ++a) {
        m_newSignature[a].second = m_oldSignature[a].second;
      }
      
      DocumentChangeSet changes;
      DocumentChangePointer change(new DocumentChange(functionContext->url(), functionContext->range(), QString(), makeSignatureString(m_newSignature, m_otherSideContext.data())));
      change->m_ignoreOldText = true;
      changes.addChange( change );
      DocumentChangeSet::ChangeResult result = changes.applyAllChanges(DocumentChangeSet::WarnOnFailedChange);
      if(!result) {
        KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
      }
    }
    
    KDevelop::DeclarationId m_otherSideId;
    KDevelop::ReferencedTopDUContext m_otherSideContext;
    QList<SignatureItem> m_oldSignature;
    QList<SignatureItem> m_newSignature;
};

void AdaptDefinitionSignatureAssistant::parseJobFinished(KDevelop::ParseJob* job) {
  if(job->document() == m_document) {
    kDebug() << "parse job finshed for current document";
    clearActions();
    KTextEditor::View* v = view();
    if(!v) {
      kDebug() << "breaking";
      return;
    }
    
    SimpleCursor currentPos(v->cursorPosition());
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    KDevelop::ReferencedTopDUContext top(DUChainUtils::standardContextForUrl(m_document.toUrl()));
    if(!top)
      return;
    
    Declaration* decl = DUChainUtils::declarationInLine(currentPos, top.data());
    if(decl && decl->identifier() == m_declarationName) {
      DUContext* context = DUChainUtils::getFunctionContext(decl);
      if(context) {
        QList<SignatureItem> newSignature;
        foreach(Declaration* parameter, context->localDeclarations()) {
          kDebug() << "parameter:" << parameter->toString() << parameter->range().textRange();
          newSignature << qMakePair(parameter->indexedType(), parameter->identifier().identifier().str());
        }
        
        KDevelop::IndexedType returnType;
        FunctionType::Ptr funType = decl->type<FunctionType>();
        if(funType)
          returnType = funType->returnType()->indexed();
        
        bool changed = false;
        if(newSignature.size() != m_oldSignature.size()){
          changed = true;
        }else{
          for(int a = 0; a < newSignature.size(); ++a)
            if(newSignature[a].first != m_oldSignature[a].first)
              changed = true;
        }

        if(changed /*|| returnType != m_oldReturnType*/) {
          kDebug() << "signature changed";
          addAction(IAssistantAction::Ptr(new AdaptSignatureAction(m_definitionId, m_definitionContext, m_oldSignature, newSignature)));
        }else{
          kDebug() << "signature stayed equal";
        }
      }
    }else{
      kDebug() << "found no declaration in line" << currentPos.textCursor() << "in document" << job->duChain();
    }
    
    emit actionsChanged();
  }
}

#include "signatureassistant.moc"