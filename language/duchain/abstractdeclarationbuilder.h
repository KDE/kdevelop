/* This file is part of KDevelop
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEV_DECLARATIONBUILDER_H
#define KDEV_DECLARATIONBUILDER_H

#include "language/duchain/classfunctiondeclaration.h"
#include "language/duchain/symboltable.h"
#include "language/duchain/forwarddeclaration.h"
#include "language/duchain/identifiedtype.h"
#include "language/duchain/functiondeclaration.h"

namespace KDevelop
{
class Declaration;

/**
 * A class which iterates the AST to extract definitions of types.
 */
template<typename T, typename NameT, typename LanguageSpecificDeclarationBuilderBase>
class KDEVPLATFORMLANGUAGE_EXPORT AbstractDeclarationBuilder : public LanguageSpecificDeclarationBuilderBase
{
protected:
  inline bool hasCurrentDeclaration() const { return !m_declarationStack.isEmpty(); }
  inline Declaration* currentDeclaration() const { return m_declarationStack.isEmpty() ? 0 : m_declarationStack.top(); }
  template<class DeclarationType>
  inline DeclarationType* currentDeclaration() const { return m_declarationStack.isEmpty() ? 0 : dynamic_cast<DeclarationType*>(m_declarationStack.top()); }

  inline const QString& comment() const { return m_lastComment; }
  inline void setComment(const QString& comment) { m_lastComment = comment; }
  inline void clearComment() { m_lastComment.clear(); }

  /**
   * Register a new declaration with the definition-use chain.
   * Returns the new declaration created.
   * @param name When this is zero, the identifier given through customName is used.
   * \param range provide a valid AST node here if name is null
   */
  virtual Declaration* openDeclaration(NameT* name, T* range, bool isFunction = false, bool isForward = false, bool isDefinition = false, bool isLocked = false)
  {
    if (!isLocked)
      DUChain::lock()->lockForWrite();

    SimpleRange newRange = editorFindRange(name ? name : range, name ? name : range);

    if (newRange.start >= newRange.end)
      kWarning() << "Range collapsed";

    QualifiedIdentifier id = identifierForNode(name);

    Identifier localId;

    if(!id.isEmpty()) {
      localId = id.last();
    }

    Declaration* declaration = 0;

    if (LanguageSpecificDeclarationBuilderBase::recompiling()) {
      // Seek a matching declaration

      // Translate cursor to take into account any changes the user may have made since the text was retrieved
      QMutexLocker smartLock(LanguageSpecificDeclarationBuilderBase::editor()->smartMutex());
      SimpleRange translated = LanguageSpecificDeclarationBuilderBase::editor()->translate(newRange);

      QList<Declaration*> declarations = LanguageSpecificDeclarationBuilderBase::currentContext()->allLocalDeclarations(localId);
      foreach( Declaration* dec, declarations ) {

        if( LanguageSpecificDeclarationBuilderBase::wasEncountered(dec) )
          continue;

        //This works because dec->textRange() is taken from a smart-range. This means that now both ranges are translated to the current document-revision.
        if (dec->range() == translated &&
            ((id.isEmpty() && dec->identifier().toString().isEmpty()) || (!id.isEmpty() && localId == dec->identifier())) &&
            dec->isDefinition() == isDefinition
            //&& extraDeclarationComparisons()
          )
        {
          if(LanguageSpecificDeclarationBuilderBase::currentContext()->type() == DUContext::Class && !dynamic_cast<ClassMemberDeclaration*>(dec))
            continue;
          /*if(isNamespaceAlias && !dynamic_cast<NamespaceAliasDeclaration*>(dec)) {
            continue;
          } else */if (isForward && !dynamic_cast<ForwardDeclaration*>(dec)) {
            continue;
          } else if (isFunction) {
            if (LanguageSpecificDeclarationBuilderBase::currentContext()->type() == DUContext::Class) {
              if (!dynamic_cast<ClassFunctionDeclaration*>(dec))
                continue;
            } else if (!dynamic_cast<AbstractFunctionDeclaration*>(dec)) {
              continue;
            }

          } else if (LanguageSpecificDeclarationBuilderBase::currentContext()->type() == DUContext::Class) {
            if (!isForward && !dynamic_cast<ClassMemberDeclaration*>(dec)) //Forward-declarations are never based on ClassMemberDeclaration
              continue;
          }

          // Match
          declaration = dec;

          // Update access policy if needed
          // updateAccessPolicy()
          break;
        }
      }
    }

    if (!declaration) {
      KTextEditor::SmartRange* prior = LanguageSpecificDeclarationBuilderBase::editor()->currentRange();
      KTextEditor::SmartRange* range = LanguageSpecificDeclarationBuilderBase::editor()->createRange(newRange.textRange());

      LanguageSpecificDeclarationBuilderBase::editor()->exitCurrentRange();
      //Q_ASSERT(range->start() != range->end());

      Q_ASSERT(LanguageSpecificDeclarationBuilderBase::editor()->currentRange() == prior);

      if (isForward) {
        declaration = new ForwardDeclaration(LanguageSpecificDeclarationBuilderBase::editor()->currentUrl(), newRange, LanguageSpecificDeclarationBuilderBase::currentContext());
      } else if (isFunction) {
        if (LanguageSpecificDeclarationBuilderBase::currentContext()->type() == DUContext::Class) {
          declaration = new ClassFunctionDeclaration(LanguageSpecificDeclarationBuilderBase::editor()->currentUrl(), newRange, LanguageSpecificDeclarationBuilderBase::currentContext());
        } else {
          declaration = new FunctionDeclaration(LanguageSpecificDeclarationBuilderBase::editor()->currentUrl(), newRange, LanguageSpecificDeclarationBuilderBase::currentContext());
        }
      } else if (LanguageSpecificDeclarationBuilderBase::currentContext()->type() == DUContext::Class) {
          declaration = new ClassMemberDeclaration(LanguageSpecificDeclarationBuilderBase::editor()->currentUrl(), newRange, LanguageSpecificDeclarationBuilderBase::currentContext());
      } else {
        declaration = new Declaration(LanguageSpecificDeclarationBuilderBase::editor()->currentUrl(), newRange, LanguageSpecificDeclarationBuilderBase::currentContext());
      }
      
      declaration->setSmartRange(range);
      declaration->setDeclarationIsDefinition(isDefinition);
      declaration->setIdentifier(localId);

      switch (LanguageSpecificDeclarationBuilderBase::currentContext()->type()) {
        case DUContext::Global:
        case DUContext::Namespace:
        case DUContext::Class:
          SymbolTable::self()->addDeclaration(declaration);
          break;
        default:
          break;
      }
    }

    declaration->setComment(m_lastComment);
    m_lastComment.clear();

    LanguageSpecificDeclarationBuilderBase::setEncountered(declaration);

    openDeclarationInternal(declaration);

    if (!isLocked)
      DUChain::lock()->releaseWriteLock();

    return declaration;
  }
  
  void openDeclarationInternal(Declaration* declaration)
  {
    m_declarationStack.push(declaration);
  }

  /// Same as the above, but sets it as the definition too
  Declaration* openDefinition(NameT* name, T* range, bool isFunction = false)
  {
    return openDeclaration(name, range, isFunction, false, true);
  }

  ForwardDeclaration* openForwardDeclaration(NameT* name, T* range)
  {
    return static_cast<ForwardDeclaration*>(openDeclaration(name, range, false, true));
  }
    
  void eventuallyAssignInternalContext()
  {
    if (LanguageSpecificDeclarationBuilderBase::lastContext()) {
      DUChainWriteLocker lock(DUChain::lock());

      if( dynamic_cast<ClassFunctionDeclaration*>(currentDeclaration()) )
        Q_ASSERT( !static_cast<ClassFunctionDeclaration*>(currentDeclaration())->isConstructor() || currentDeclaration()->context()->type() == DUContext::Class );

      if(LanguageSpecificDeclarationBuilderBase::lastContext() && (LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Class || LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Other || LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Function || LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Template || LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Enum) )
      {
        if( !LanguageSpecificDeclarationBuilderBase::lastContext()->owner() || !LanguageSpecificDeclarationBuilderBase::wasEncountered(LanguageSpecificDeclarationBuilderBase::lastContext()->owner()) ) { //if the context is already internalContext of another declaration, leave it alone
          currentDeclaration()->setInternalContext(LanguageSpecificDeclarationBuilderBase::lastContext());

          LanguageSpecificDeclarationBuilderBase::clearLastContext();
        }
      }
    }
  }
    
  virtual void closeDeclaration()
  {
    m_declarationStack.pop();
  }
    
  void abortDeclaration()
  {
    m_declarationStack.pop();
  }

private:
  QStack<Declaration*> m_declarationStack;
  QString m_lastComment;
};

}

#endif // KDEV_DECLARATIONBUILDER_H
