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

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/declaration.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/functiondefinition.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/types/arraytype.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KLocalizedString>
#include <KMessageBox>

#include "cppduchain.h"

using namespace  KDevelop;
using namespace Cpp;

AdaptDefinitionSignatureAssistant::AdaptDefinitionSignatureAssistant(KTextEditor::View* view,
                                                                     const KTextEditor::Range& inserted)
: m_editingDefinition(false)
, m_view(view)
{
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
  m_oldSignature.isConst = otherSide->abstractType() && otherSide->abstractType()->modifiers() & AbstractType::ConstModifier;

  KDevelop::FunctionType::Ptr funType = otherSide->type<KDevelop::FunctionType>();
  if(funType)
    m_oldSignature.returnType = funType->returnType()->indexed();

  //Schedule an update, to make sure the ranges match
  DUChain::self()->updateContextForUrl(m_definitionContext->url(), KDevelop::TopDUContext::AllDeclarationsAndContexts);

}

bool AdaptDefinitionSignatureAssistant::isUseful() {
  kDebug() << m_declarationName.toString() << m_definitionId.qualifiedIdentifier().toString();
  return !m_declarationName.isEmpty() && m_definitionId.isValid();
}

DUContext* AdaptDefinitionSignatureAssistant::findFunctionContext(const KUrl& url,
                                                                  const KDevelop::SimpleRange& _range) const {
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  TopDUContext* top = DUChainUtils::standardContextForUrl(url);

  if(top) {
    RangeInRevision range = top->transformToLocalRevision(_range);
    DUContext* context = top->findContextAt(range.start, true);
    if(context == top)
      context = top->findContextAt(range.end, true);

    if(context && context->type() == DUContext::Function && context->owner()) {
      return context;
    }
  }
  return 0;
}

QString makeSignatureString(const Signature& signature, DUContext* visibilityFrom) {
  QString ret;
  int pos = 0;
  foreach(const ParameterItem& item, signature.parameters) {
    if(!ret.isEmpty())
      ret += ", ";

    ///TODO: merge common code with helpers.cpp::createArgumentList
    AbstractType::Ptr type = item.first.abstractType();

    QString arrayAppendix;
    ArrayType::Ptr arrayType;
    while ((arrayType = type.cast<ArrayType>())) {
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
    AdaptSignatureAction(const KDevelop::DeclarationId& definitionId,
                         KDevelop::ReferencedTopDUContext definitionContext,
                         const Signature& oldSignature,
                         const Signature& newSignature,
                         bool editingDefinition)
    : m_otherSideId(definitionId),
    m_otherSideContext(definitionContext),
    m_oldSignature(oldSignature),
    m_newSignature(newSignature),
    m_editingDefinition(editingDefinition) {
    }

    virtual QString description() const {
      return m_editingDefinition ? i18n("Update declaration signature") : i18n("Update definition signature");
    }

    virtual QString toolTip() const {
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
      return i18n("Update %1\nfrom: %2(%3)%4\nto: %2(%5)%6",
                  m_editingDefinition ? i18n("declaration") : i18n("definition"),
                  m_otherSideId.qualifiedIdentifier().toString(),
                  makeSignatureString(m_oldSignature, m_otherSideContext.data()),
                  m_oldSignature.isConst ? " const" : "",
                  makeSignatureString(m_newSignature, m_otherSideContext.data()),
                  m_newSignature.isConst ? " const" : "");
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
      DocumentChange changeParameters(functionContext->url(), functionContext->rangeInCurrentRevision(), QString(), makeSignatureString(m_newSignature, m_otherSideContext.data()));
      changeParameters.m_ignoreOldText = true;
      changes.addChange( changeParameters );
      if (m_oldSignature.isConst != m_newSignature.isConst) {
        ///TODO: also use code representation here
        RangeInRevision range = functionContext->range();
        // go after closing paren
        range.end.column++;
        // start == end (default when not const before)
        range.start = range.end;
        QString oldText;
        QString newText;
        if (m_oldSignature.isConst) {
          range.end.column += 6;
          oldText = " const";
        } else {
          newText = " const";
        }
        DocumentChange changeConstness(functionContext->url(), range.castToSimpleRange(), oldText, newText);
        changes.addChange(changeConstness);
      }
      if (m_oldSignature.returnType != m_newSignature.returnType) {
        CodeRepresentation::Ptr document = createCodeRepresentation(functionContext->url());
        int l = functionContext->range().start.line;
        QString line = document->line(l);
        QRegExp exe( QString("^(\\s*)(.+)\\s+(?:\\w+::)*\\b%1\\s*\\(").arg(otherSide->identifier().toString()), Qt::CaseSensitive, QRegExp::RegExp2 );
        int pos = exe.indexIn(line);
        if (pos != -1) {
          QString oldText = exe.cap(2);
          SimpleRange range = SimpleRange(l, exe.cap(1).length(), l, exe.cap(1).length() + oldText.length());
          QString newText = Cpp::simplifiedTypeString(m_newSignature.returnType.abstractType(), functionContext->parentContext());
          DocumentChange changeRetType(functionContext->url(), range, oldText, newText);
          changes.addChange(changeRetType);
        }
      }
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
    bool m_editingDefinition;
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
    if(!m_view) {
      kDebug() << "breaking";
      return;
    }
    SimpleCursor currentPos(m_view->cursorPosition());
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
        newSignature.isConst = decl->abstractType() && decl->abstractType()->modifiers() & AbstractType::ConstModifier;

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

        if(newSignature.isConst != m_oldSignature.isConst)
          changed = true;

        //We only need to fiddle around with default parameters if we're updating the declaration
        if(!m_editingDefinition) {
          for(QList<QString>::iterator it = newSignature.defaultParams.begin(); it != newSignature.defaultParams.end(); ++it)
            *it = QString();
        }

        changed = changed || newSignature.returnType != m_oldSignature.returnType;

        if(changed) {
          kDebug() << "signature changed";
          addAction(IAssistantAction::Ptr(new AdaptSignatureAction(m_definitionId, m_definitionContext, m_oldSignature, newSignature, m_editingDefinition)));
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