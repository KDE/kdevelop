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

#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

namespace KDevelop {

/**
 * A class which iterates the AST to extract uses of definitions.
 */
template<typename T, typename NameT, typename LanguageSpecificUseBuilderBase>
class KDEVPLATFORMLANGUAGE_EXPORT AbstractUseBuilder: public LanguageSpecificUseBuilderBase
{
public:
  AbstractUseBuilder()
    : m_finishContext(true)
  {
  }

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   */
  void buildUses(T *node)
  {
    TopDUContext* top = dynamic_cast<TopDUContext*>(contextFromNode(node));

    if (top) {
      DUChainWriteLocker lock(DUChain::lock());
      top->clearDeclarationIndices();
      if(top->hasUses())
        LanguageSpecificUseBuilderBase::setRecompiling(true);
    }

    LanguageSpecificUseBuilderBase::supportBuild(node);

    if (top)
      top->setHasUses(true);
  }

protected:
  /**
   * @param decl May be zero for not found declarations
   * */
  /// Register a new use
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
  
  void newUse(SimpleRange newRange, Declaration* declaration)
  {
    DUChainWriteLocker lock(DUChain::lock());
    if(!declaration) {
      kDebug() << "Tried to create use of zero declaration";
      return;
    }

    /**
    * We need to find a context that this use fits into, which must not necessarily be the current one.
    * The reason are macros like SOME_MACRO(SomeClass), where SomeClass is expanded to be within a
    * sub-context that comes from the macro. That sub-context will have a very small range, and will most
    * probably not be the range of the actual "SomeClass" text, so the "SomeClass" use has to be moved
    * into the context that surrounds the SOME_MACRO invocation.
    * */
    DUContext* newContext = LanguageSpecificUseBuilderBase::currentContext();
    int contextUpSteps = 0; //We've got to use the stack here, and not parentContext(), because the order may be different
    while (!newContext->range().contains(newRange) && contextUpSteps < (LanguageSpecificUseBuilderBase::contextStack().size()-1)) {
      ++contextUpSteps;
      newContext = LanguageSpecificUseBuilderBase::contextStack()[LanguageSpecificUseBuilderBase::contextStack().size()-1-contextUpSteps];
    }

    if (contextUpSteps) {
      LanguageSpecificUseBuilderBase::editor()->setCurrentRange(newContext->smartRange()); //We have to do this, because later we will call closeContext(), and that will close one smart-range
      m_finishContext = false;
      openContext(newContext);
      m_finishContext = true;
      nextUseIndex() = m_nextUseStack.at(m_nextUseStack.size()-contextUpSteps-2);
      skippedUses() = m_skippedUses.at(m_skippedUses.size()-contextUpSteps-2);

      Q_ASSERT(m_contexts[m_nextUseStack.size()-contextUpSteps-2] == LanguageSpecificUseBuilderBase::currentContext());
      Q_ASSERT(LanguageSpecificUseBuilderBase::currentContext()->uses().count() >= nextUseIndex());
    }

    bool encountered = false;

    int declarationIndex = LanguageSpecificUseBuilderBase::currentContext()->topContext()->indexForUsedDeclaration(declaration);

    if (LanguageSpecificUseBuilderBase::recompiling()) {

      const QVector<Use>& uses = LanguageSpecificUseBuilderBase::currentContext()->uses();
      // Translate cursor to take into account any changes the user may have made since the text was retrieved
      SimpleRange translated = newRange;
      if (LanguageSpecificUseBuilderBase::editor()->smart()) {
        QMutexLocker smartLock(LanguageSpecificUseBuilderBase::editor()->smart()->smartMutex());
        translated = SimpleRange(LanguageSpecificUseBuilderBase::editor()->smart()->translateFromRevision(translated.textRange()) );
      }

      for (; nextUseIndex() < uses.count(); ++nextUseIndex()) {
        const Use& use = uses[nextUseIndex()];

        //Thanks to the preprocessor, it's possible that uses are created in a wrong order. We do this anyway.
        if (use.m_range.start > translated.end && LanguageSpecificUseBuilderBase::editor()->smart() )
          break;

        if (use.m_range == translated)
        {
          LanguageSpecificUseBuilderBase::currentContext()->setUseDeclaration(nextUseIndex(), declarationIndex);
          ++nextUseIndex();
          // Match
          encountered = true;

          break;
        }
        //Not encountered, and before the current range. Remove all intermediate uses.
        skippedUses().append(nextUseIndex());
      }
    }

    if (!encountered) {

      KTextEditor::SmartRange* use = LanguageSpecificUseBuilderBase::editor()->createRange(newRange.textRange());
      LanguageSpecificUseBuilderBase::editor()->exitCurrentRange();

      LanguageSpecificUseBuilderBase::currentContext()->createUse(declarationIndex, newRange, use, nextUseIndex());
      ++nextUseIndex();
    }

    if (contextUpSteps) {
      Q_ASSERT(m_contexts[m_nextUseStack.size()-contextUpSteps-2] == LanguageSpecificUseBuilderBase::currentContext());
      Q_ASSERT(LanguageSpecificUseBuilderBase::currentContext()->uses().count() >= nextUseIndex());
      m_nextUseStack[m_nextUseStack.size()-contextUpSteps-2] = nextUseIndex();
      m_skippedUses[m_skippedUses.size()-contextUpSteps-2] = skippedUses();
      m_finishContext = false;
      closeContext();
      m_finishContext = true;
    }
  }

  virtual void openContext(KDevelop::DUContext* newContext)
  {
    LanguageSpecificUseBuilderBase::openContext(newContext);

    m_contexts.push(newContext);
    m_nextUseStack.push(0);
    m_skippedUses.push(QVector<int>());
  }
  
  virtual void closeContext()
  {
    if(m_finishContext) {
      DUChainWriteLocker lock(DUChain::lock());

      //Delete all uses that were not encountered
      //That means: All uses in skippedUses, and all uses from nextUseIndex() to LanguageSpecificUseBuilderBase::currentContext()->uses().count()
      for(int a = LanguageSpecificUseBuilderBase::currentContext()->uses().count()-1; a >= nextUseIndex(); --a)
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

