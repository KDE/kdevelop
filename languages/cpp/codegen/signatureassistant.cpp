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
#include <language/duchain/types/arraytype.h>

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
  
//   kDebug() << "checking";
  
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
//     kDebug() << "no declaration found in line";
    return;
  }
  
  m_declarationName = funDecl->identifier();
//   kDebug() << "found local function declaration:" << m_declarationName.toString();
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
    m_oldSignature.defaultParams << otherFunDecl->defaultParameterForArgument(pos).str();
    m_oldSignature.parameters << qMakePair(parameter->indexedType(), parameter->identifier().identifier().str());
    ++pos;
  }
  
  KDevelop::FunctionType::Ptr funType = otherSide->type<KDevelop::FunctionType>();
  if(funType)
    m_oldSignature.returnType = funType->returnType()->indexed();
  
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

QString makeSignatureString(Signature signature, DUContext* visibilityFrom) {
  QString ret;
  int pos = 0;
  foreach(const ParameterItem& item, signature.parameters) {
    if(!ret.isEmpty())
      ret += ", ";
    
    ///TODO: merge common code with helpers.cpp::createArgumentList
    AbstractType::Ptr type = item.first.abstractType();

    QString arrayAppendix;
    ArrayType::Ptr arrayType;
    while (arrayType = type.cast<ArrayType>()) {
      type = arrayType->elementType();
      //note: we have to prepend since we iterate from outside, i.e. from right to left.
      if (arrayType->dimension()) {
        arrayAppendix.prepend(QString("[%1]").arg(arrayType->dimension()));
      } else {
        // dimensionless
        arrayAppendix.prepend("[]");
      }
    }
    ret += Cpp::simplifiedTypeString(type,  visibilityFrom);
    
    if(!item.second.isEmpty())
      ret += " " + item.second;
    
    ret += arrayAppendix;
    
    if (signature.defaultParams.size() > pos && !signature.defaultParams[pos].isEmpty())
      ret += " = " + signature.defaultParams[pos];
    
    ++pos;
  }
  return ret;
}

class AdaptSignatureAction : public KDevelop::IAssistantAction {
  public:
    AdaptSignatureAction(KDevelop::DeclarationId definitionId, KDevelop::ReferencedTopDUContext definitionContext, Signature oldSignature, Signature newSignature) : 
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
      ///@todo Eventually do real refactoring-like renaming of the function?
      if(!functionContext || functionContext->type() != DUContext::Function) {
        kDebug() << "no correct function context";
        return;
      }
      
      DocumentChangeSet changes;
      DocumentChange changeParameters(functionContext->url(), functionContext->range(), QString(), makeSignatureString(m_newSignature, m_otherSideContext.data()));
      changeParameters.m_ignoreOldText = true;
      changes.addChange( changeParameters );
      changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
      DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
      if(!result) {
        KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
      }
    }
    
    KDevelop::DeclarationId m_otherSideId;
    KDevelop::ReferencedTopDUContext m_otherSideContext;
    Signature m_oldSignature;
    Signature m_newSignature;
};

/* -- only needed for return type updating, which isn't supported yet
static QString typeToString(KDevelop::IndexedType type) {
  KDevelop::AbstractType::Ptr t = type.abstractType();
  if(t)
    return t->toString();
  else
    return QString();
}*/

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
        int pos = 0;
        Signature newSignature;
        AbstractFunctionDeclaration* contextFunction = dynamic_cast<AbstractFunctionDeclaration*>(decl);
        foreach(Declaration* parameter, context->localDeclarations()) {
//           kDebug() << "parameter:" << parameter->toString() << parameter->range().textRange();
          newSignature.defaultParams << contextFunction->defaultParameterForArgument(pos).str();
          newSignature.parameters << qMakePair(parameter->indexedType(), parameter->identifier().identifier().str());
          ++pos;
        }
        
        KDevelop::IndexedType newReturnType;
        FunctionType::Ptr funType = decl->type<FunctionType>();
        if(funType)
          newSignature.returnType = funType->returnType()->indexed();
        
        bool changed = false;
        bool canHaveDefault = false;
        for (int curNewParam = newSignature.parameters.size() - 1; curNewParam >= 0 ; --curNewParam)
        {//detect changes in parameters, assign default arguments as needed
          int foundAt = -1;
          canHaveDefault = canHaveDefault | (curNewParam == newSignature.parameters.size() - 1);
          
          for (int curOldParam = m_oldSignature.parameters.size() - 1; curOldParam >= 0 ; --curOldParam) {
            if (newSignature.parameters[curNewParam].first == m_oldSignature.parameters[curOldParam].first) {
               if (newSignature.parameters[curNewParam].second == m_oldSignature.parameters[curOldParam].second ||
                   curOldParam == curNewParam) {
                //given the same type and either the same position or the same name, it's (probably) the same argument
                foundAt = curOldParam;
                
                if (newSignature.parameters[curNewParam].second != m_oldSignature.parameters[curOldParam].second ||
                    curOldParam != curNewParam)
                  changed = true; //Either the name changed at this position, or position of this name has changed
                
                if (newSignature.parameters[curNewParam].second == m_oldSignature.parameters[curOldParam].second)
                  break; //Found an argument with the same name and type, no need to look further
                //else: position/type match, but name match will trump, allowing: (int i=0, int j=1) => (int j=1, int i=0)
              }
            }
          }
          
          if (foundAt < 0)
            changed = true;
          else if (!m_oldSignature.defaultParams[foundAt].isEmpty() &&
                    newSignature.defaultParams[curNewParam].isEmpty()) {
            //the other side's arg specified a default, this side didn't, preserve old default for this arg
            if (canHaveDefault)
              newSignature.defaultParams[curNewParam] = m_oldSignature.defaultParams[foundAt];
          } else {
            canHaveDefault = false; //This param didn't have a default, none that follow may either
          }
        }
        
        if(newSignature.parameters.size() != m_oldSignature.parameters.size())
          changed = true;
        
        //We only need to fiddle around with default parameters if we're updating the declaration
        if(!m_editingDefinition) {
          for(QList<QString>::iterator it = newSignature.defaultParams.begin(); it != newSignature.defaultParams.end(); ++it)
            *it = QString();
        }
          
        if(changed /*|| newSignature.returnType != m_oldSignature.returnType*/) {
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