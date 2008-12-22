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

#include "./implementationhelperitem.h"
#include <ktexteditor/document.h>
#include <klocalizedstring.h>
#include <language/duchain/duchainutils.h>
#include "completionhelpers.h"
#include <language/duchain/types/functiontype.h>
#include <language/duchain/classfunctiondeclaration.h>

ImplementationHelperItem::ImplementationHelperItem(HelperType type, KDevelop::DeclarationPointer decl, KSharedPtr<Cpp::CodeCompletionContext> context, int _inheritanceDepth, int _listOffset) : NormalDeclarationCompletionItem(decl, context, _inheritanceDepth, _listOffset), m_type(type) {
}

QVariant ImplementationHelperItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const {
  QVariant ret = NormalDeclarationCompletionItem::data(index, role, model);
  if(index.column() == KTextEditor::CodeCompletionModel::Prefix && role == Qt::DisplayRole) {
    QString prefix;
    if(m_type == Override)
      prefix = i18n("Override");
    if(m_type == CreateDefinition)
      prefix = i18n("Implement");
    
    ret = prefix + " " + ret.toString();
  }
  if(role == KTextEditor::CodeCompletionModel::InheritanceDepth)
    return QVariant(0);
  return ret;
}

QString ImplementationHelperItem::signaturePart(bool includeDefaultParams) {
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  QString ret;
  createArgumentList(*this, ret, 0, false);
  if(m_declaration->abstractType() && m_declaration->abstractType()->modifiers() & AbstractType::ConstModifier)
    ret += " const";
  return ret;
}

void ImplementationHelperItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  
  QString newText;
  if(!m_declaration)
    return;

  if(m_type == Override) {
    if(!useAlternativeText) {
      if(m_declaration) {
        newText = "virtual ";
        FunctionType::Ptr asFunction = m_declaration->type<FunctionType>();
        if(asFunction && asFunction->returnType())
            newText += asFunction->returnType()->toString() + " ";
        
        newText += m_declaration->identifier().toString();
        newText += signaturePart(true);
        newText += ";";
      } else {
        kDebug() << "Declaration disappeared";
        return;
      }
    }else{
      newText = alternativeText;
    }
  }else if(m_type == CreateDefinition) {
      QualifiedIdentifier localScope;
      TopDUContext* topContext = DUChainUtils::standardContextForUrl(document->url());
      if(topContext) {
        DUContext* context = topContext->findContextAt(SimpleCursor(word.end()));
        if(context)
          localScope = context->scopeIdentifier(true);
      }
      
      QualifiedIdentifier scope = m_declaration->qualifiedIdentifier();
      
      if(scope.count() <= localScope.count() || !scope.toString().startsWith(localScope.toString()))
        return;
      
      scope = scope.mid( localScope.count() );
      
      FunctionType::Ptr asFunction = m_declaration->type<FunctionType>();
      
      if(asFunction && asFunction->returnType())
          newText += asFunction->returnType()->toString() + " ";
      newText += scope.toString();
      newText += signaturePart(false);
      newText += " {\n";
      
      if(asFunction) {
      
        ClassFunctionDeclaration* overridden = dynamic_cast<ClassFunctionDeclaration*>(DUChainUtils::getOverridden(m_declaration.data()));
        if(overridden && !overridden->isAbstract()) {
          if(asFunction->returnType() && asFunction->returnType()->toString() != "void") {
            newText += "return ";
          }
          QualifiedIdentifier baseScope = overridden->qualifiedIdentifier();
          bool foundShorter = true;
          do {
            foundShorter = false;
            QualifiedIdentifier candidate = baseScope;
            if(candidate.count() > 2) {
              candidate.pop();
              QList<Declaration*> decls = m_declaration->context()->findDeclarations(candidate);
              if(decls.contains(overridden)) {
                foundShorter = true;
                baseScope = candidate;
              }
            }
          }while(foundShorter);
          
          newText += baseScope.toString() + "(";
          
          DUContext* ctx = m_declaration->internalContext();
          if(ctx->type() == DUContext::Function) {
            bool first = true;
            foreach(Declaration* decl, ctx->localDeclarations()) {
              if(!first)
                newText += ", ";
              first = false;
              newText += decl->identifier().toString();
            }
          }
          
          newText += ");";
        }
      }
      
      newText += "\n}\n";
  }

  lock.unlock();
  
  document->replaceText(word, newText);
}

