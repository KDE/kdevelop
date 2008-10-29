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

#ifndef KDEV_USEBUILDER_H
#define KDEV_USEBUILDER_H

#include "../declaration.h"
#include "../use.h"
#include "../topducontext.h"
#include "../duchain.h"
#include "../duchainlock.h"

#include <language/editor/editorintegrator.h>

namespace KDevelop {

/**
 * \short Abstract definition-use chain use builder class
 *
 * The AbstractUseBuilder is a convenience class template for creating customized
 * definition-use chain use builders from an AST.  It simplifies:
 * - use of your editor integrator
 * - creating or modifying existing \ref Use "Uses"
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
template<typename T, typename NameT, typename LanguageSpecificUseBuilderBase>
class AbstractUseBuilder: public LanguageSpecificUseBuilderBase
{
public:
  /// Constructor.
  AbstractUseBuilder()
    : m_finishContext(true)
  {
  }

  /**
   * Iterate an existing duchain, and add, remove or modify uses as determined
   * from the ast.
   *
   * \param node AST node to start visiting.
   */
  void buildUses(T *node)
  {
    TopDUContext* top = dynamic_cast<TopDUContext*>(contextFromNode(node));

    if (top) {
      DUChainWriteLocker lock(DUChain::lock());
      top->clearUsedDeclarationIndices();
      if(top->features() & TopDUContext::AllDeclarationsContextsAndUses)
        LanguageSpecificUseBuilderBase::setRecompiling(true);
    }

    LanguageSpecificUseBuilderBase::supportBuild(node);
  }

protected:
  /**
   * Register a new use at the AST node \a name.
   *
   * \param node AST node which both represents a use and the identifier for the declaration which is being used.
   */
  void newUse(NameT* name)
  {
    QualifiedIdentifier id = identifierForNode(name);

    SimpleRange newRange = editorFindRange(name, name);

    DUChainWriteLocker lock(DUChain::lock());
    QList<Declaration*> declarations = LanguageSpecificUseBuilderBase::currentContext()->findDeclarations(id, newRange.start);
    foreach (Declaration* declaration, declarations)
      if (!declaration->isForwardDeclaration()) {
        declarations.clear();
        declarations.append(declaration);
        break;
      }
    // If we don't break, there's no non-forward declaration

    lock.unlock();
    newUse( newRange, !declarations.isEmpty() ? declarations.first() : 0 );
  }

  /**
   * Register a new use for a \a declaration with a \a node.
   *
   * \param node Node which encompasses the use.
   * \param decl Declaration which is being used. May be null when a declaration cannot be found for the use.
   */
  void newUse(T* node, KDevelop::Declaration* declaration)
  {
    newUse(editorFindRange(node, node), declaration);
  }

  /**
   * Register a new use.
   *
   * \param newRange Text range which encompasses the use.
   * \param decl Declaration which is being used. May be null when a declaration cannot be found for the use.
   */
  void newUse(SimpleRange newRange, Declaration* declaration)
  {
    DUChainWriteLocker lock(DUChain::lock());
    if(!declaration) {
      kDebug() << "Tried to create use of zero declaration";
      return;
    }

    bool encountered = false;
    int declarationIndex = LanguageSpecificUseBuilderBase::currentContext()->topContext()->indexForUsedDeclaration(declaration);
    int contextUpSteps = 0; //We've got to use the stack here, and not parentContext(), because the order may be different

    {
      //We've got to consider the translated range, and while we use it, the smart-mutex needs to be locked
      LockedSmartInterface iface = LanguageSpecificUseBuilderBase::editor()->smart();
      SimpleRange translated = LanguageSpecificUseBuilderBase::editor()->translate(iface, newRange);

      /*
      * We need to find a context that this use fits into, which must not necessarily be the current one.
      * The reason are macros like SOME_MACRO(SomeClass), where SomeClass is expanded to be within a
      * sub-context that comes from the macro. That sub-context will have a very small range, and will most
      * probably not be the range of the actual "SomeClass" text, so the "SomeClass" use has to be moved
      * into the context that surrounds the SOME_MACRO invocation.
      * */
      DUContext* newContext = LanguageSpecificUseBuilderBase::currentContext();
      while (!newContext->range().contains(translated) && contextUpSteps < (LanguageSpecificUseBuilderBase::contextStack().size()-1)) {
        ++contextUpSteps;
        newContext = LanguageSpecificUseBuilderBase::contextStack()[LanguageSpecificUseBuilderBase::contextStack().size()-1-contextUpSteps];
      }

      if (contextUpSteps) {
        LanguageSpecificUseBuilderBase::editor()->setCurrentRange(iface, newContext->smartRange()); //We have to do this, because later we will call closeContext(), and that will close one smart-range
        m_finishContext = false;
        openContext(newContext);
        m_finishContext = true;
        nextUseIndex() = m_nextUseStack.at(m_nextUseStack.size()-contextUpSteps-2);
        skippedUses() = m_skippedUses.at(m_skippedUses.size()-contextUpSteps-2);

        Q_ASSERT(m_contexts[m_nextUseStack.size()-contextUpSteps-2] == LanguageSpecificUseBuilderBase::currentContext());
        Q_ASSERT(LanguageSpecificUseBuilderBase::currentContext()->usesCount() >= nextUseIndex());
      }

      if (LanguageSpecificUseBuilderBase::recompiling()) {

        const Use* uses = LanguageSpecificUseBuilderBase::currentContext()->uses();
        // Translate cursor to take into account any changes the user may have made since the text was retrieved

        for (; nextUseIndex() < LanguageSpecificUseBuilderBase::currentContext()->usesCount(); ++nextUseIndex()) {
          const Use& use = uses[nextUseIndex()];

          //Thanks to the preprocessor, it's possible that uses are created in a wrong order. We do this anyway.
          if (use.m_range.start > translated.end && LanguageSpecificUseBuilderBase::editor()->smart() ) {
#ifdef DEBUG_UPDATE_MATCHING
            kDebug() << "use of" << (declaration ? declaration->qualifiedIdentifier().toString() : QString()) << "with range" << translated.textRange() << "found use of" << (LanguageSpecificUseBuilderBase::currentContext()->topContext()->usedDeclarationForIndex(use.m_declarationIndex) ? LanguageSpecificUseBuilderBase::currentContext()->topContext()->usedDeclarationForIndex(use.m_declarationIndex)->qualifiedIdentifier().toString() : QString()) << "with range" << use.m_range.textRange() << ", stopping";
#endif
            break;
          }

          if (use.m_range == translated)
          {
            LanguageSpecificUseBuilderBase::currentContext()->setUseDeclaration(nextUseIndex(), declarationIndex);
            ++nextUseIndex();
            // Match
            encountered = true;

            break;
          }
#ifdef DEBUG_UPDATE_MATCHING
            kDebug() << "use of" << (declaration ? declaration->qualifiedIdentifier().toString() : QString()) << "with range" << translated.textRange() << "skipping use of" << (LanguageSpecificUseBuilderBase::currentContext()->topContext()->usedDeclarationForIndex(use.m_declarationIndex) ? LanguageSpecificUseBuilderBase::currentContext()->topContext()->usedDeclarationForIndex(use.m_declarationIndex)->qualifiedIdentifier().toString() : QString()) << "with range" << use.m_range.textRange();
#endif
          //Not encountered, and before the current range. Remove all intermediate uses.
          skippedUses().append(nextUseIndex());
        }
      }
    }

    if (!encountered) {
      LockedSmartInterface iface = LanguageSpecificUseBuilderBase::editor()->smart();
      KTextEditor::SmartRange* use = LanguageSpecificUseBuilderBase::editor()->currentRange(iface) ? LanguageSpecificUseBuilderBase::editor()->createRange(iface, newRange.textRange()) : 0;
      LanguageSpecificUseBuilderBase::editor()->exitCurrentRange(iface);

      LanguageSpecificUseBuilderBase::currentContext()->createUse(declarationIndex, newRange, use, nextUseIndex());
      ++nextUseIndex();
    }

    if (contextUpSteps) {
      Q_ASSERT(m_contexts[m_nextUseStack.size()-contextUpSteps-2] == LanguageSpecificUseBuilderBase::currentContext());
      Q_ASSERT(LanguageSpecificUseBuilderBase::currentContext()->usesCount() >= nextUseIndex());
      m_nextUseStack[m_nextUseStack.size()-contextUpSteps-2] = nextUseIndex();
      m_skippedUses[m_skippedUses.size()-contextUpSteps-2] = skippedUses();
      m_finishContext = false;
      closeContext();
      m_finishContext = true;
    }
  }

  /**
   * Reimplementation of openContext, to track which uses should be assigned to which context.
   */
  virtual void openContext(KDevelop::DUContext* newContext)
  {
    LanguageSpecificUseBuilderBase::openContext(newContext);

    m_contexts.push(newContext);
    m_nextUseStack.push(0);
    m_skippedUses.push(QVector<int>());
  }

  /**
   * Reimplementation of closeContext, to track which uses should be assigned to which context.
   */
  virtual void closeContext()
  {
    if(m_finishContext) {
      DUChainWriteLocker lock(DUChain::lock());

      //Delete all uses that were not encountered
      //That means: All uses in skippedUses, and all uses from nextUseIndex() to LanguageSpecificUseBuilderBase::currentContext()->usesCount()
      for(int a = LanguageSpecificUseBuilderBase::currentContext()->usesCount()-1; a >= nextUseIndex(); --a)
        LanguageSpecificUseBuilderBase::currentContext()->deleteUse(a);
      for(int a = skippedUses().count()-1; a >= 0; --a)
        LanguageSpecificUseBuilderBase::currentContext()->deleteUse(skippedUses()[a]);
    }

    LanguageSpecificUseBuilderBase::closeContext();

    m_contexts.pop();
    m_nextUseStack.pop();
    m_skippedUses.pop();
  }

private:
  inline int& nextUseIndex() { return m_nextUseStack.top(); }
  inline QVector<int>& skippedUses() { return m_skippedUses.top(); }
  QStack<int> m_nextUseStack;
  QStack<QVector<int> > m_skippedUses;
  QStack<KDevelop::DUContext*> m_contexts;

  //Whether not encountered uses should be deleted during closeContext()
  bool m_finishContext;
};

}

#endif // USEBUILDER_H

