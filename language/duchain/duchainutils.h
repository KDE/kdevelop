/*
 * DUChain Utilities
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_DUCHAINUTILS_H
#define KDEVPLATFORM_DUCHAINUTILS_H

#include <KDE/KIcon>

#include <ktexteditor/codecompletionmodel.h>

#include "../languageexport.h"

class KUrl;

namespace KDevelop {

class Declaration;
class DUChainBase;
class DUContext;
class SimpleCursor;
class IndexedString;
class TopDUContext;
class IndexedDUContext;
class IndexedDeclaration;

/**
 * A namespace which contains convenience utilities for navigating definition-use chains.
 */
namespace DUChainUtils {
  KDEVPLATFORMLANGUAGE_EXPORT KTextEditor::CodeCompletionModel::CompletionProperties completionProperties(const Declaration* dec);
  KDEVPLATFORMLANGUAGE_EXPORT QIcon iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p);
  KDEVPLATFORMLANGUAGE_EXPORT QIcon iconForDeclaration(const Declaration* dec);
  /** Asks the language-plugins for standard-contexts for the given url, and returns one if available.
    * If there is no language-plugin registered for the given url, it will just try to get any top-context for the file from the du-chain.
    * NOTE: The DUChain needs to be read or write locked when you call this.
    * @param proxyContext Whether the returned context should be a proxy context. When no proxy-context is found, a normal context is returned.
    *
    * FIXME: this should operate on IndexedString
    */
  KDEVPLATFORMLANGUAGE_EXPORT KDevelop::TopDUContext* standardContextForUrl(const KUrl& url, bool preferProxyContext = false);
  /**
   * Returns the content-context associated to the given proxy-contex.
   * Returns the same context if it is not a proxy-context.
   * Returns zero if no content-context could be acquired.
   * */
  KDEVPLATFORMLANGUAGE_EXPORT TopDUContext* contentContextFromProxyContext(TopDUContext* top);
  /** Returns the Declaration/Definition under the cursor, or zero. DUChain does not need to be locked.
   * Must only be called from the foreground or with the foreground lock held.
   * If the item under the cursor is a use, the declaration is returned. */
  KDEVPLATFORMLANGUAGE_EXPORT Declaration* itemUnderCursor(const KUrl& url, const SimpleCursor& cursor);
  /** Returns the Declaration/Definition/Use range under the cursor or empty range.
   * DUChain must be locked.
   * Must only be called from the foreground or with the foreground lock held. */
  KDEVPLATFORMLANGUAGE_EXPORT KTextEditor::Range itemRangeUnderCursor(const KUrl& url, const SimpleCursor& cursor);
  /**If the given declaration is a definition, and has a real declaration
    *attached, returns that declarations. Else returns the given argument. */
  KDEVPLATFORMLANGUAGE_EXPORT Declaration* declarationForDefinition(Declaration* definition, TopDUContext* topContext = 0);
  ///Returns the first declaration in the given line. Searches the given context and all sub-contexts.
  ///Must only be called from the foreground or with the foreground lock held.
  KDEVPLATFORMLANGUAGE_EXPORT Declaration* declarationInLine(const KDevelop::SimpleCursor& cursor, KDevelop::DUContext* ctx);

  class KDEVPLATFORMLANGUAGE_EXPORT DUChainItemFilter {
    public:
    virtual bool accept(Declaration* decl) = 0;
    //Should return whether processing should be deepened into the given context
    virtual bool accept(DUContext* ctx) = 0;
    virtual ~DUChainItemFilter();
  };
  ///walks a context, all its sub-contexts, and all its declarations in exactly the order they appear in in the file.
  ///Re-implement DUChainItemFilter to do something with the items.
  KDEVPLATFORMLANGUAGE_EXPORT void collectItems( DUContext* context, DUChainItemFilter& filter );

  KDEVPLATFORMLANGUAGE_EXPORT DUContext* getArgumentContext(Declaration* decl);

  ///Uses the persistent symbol table to find all occurrences of this declaration, based on its identifier.
  ///The result should be filtered to make sure that the declaration is actually useful to you.
  KDEVPLATFORMLANGUAGE_EXPORT QList<IndexedDeclaration> collectAllVersions(Declaration* decl);

  ///If the given declaration is a class, this gets all classes that inherit this one
  ///@param collectVersions If this is true, the persistent symbol table is used to first find all registered
  ///                       versions of this class, and then get the inheriters from them all together. This is needed for C++.
  ///@param maxAllowedSteps The maximum of steps allowed. If this is zero in the end, this means the search has been stopped with the max. reached
  ///                                           If you really want _all_ inheriters, you should initialize it with a very large value.
  KDEVPLATFORMLANGUAGE_EXPORT QList<Declaration*> getInheriters(const Declaration* decl, uint& maxAllowedSteps, bool collectVersions = true);

  ///Gets all functions that override the function @param overriddenDeclaration, starting the search at @param currentClass
  ///@param maxAllowedSteps The maximum of steps allowed. If this is zero in the end, this means the search has been stopped with the max. reached
  KDEVPLATFORMLANGUAGE_EXPORT QList<Declaration*> getOverriders(const Declaration* currentClass, const Declaration* overriddenDeclaration, uint& maxAllowedSteps);

  ///Returns whether the given context or any of its child-contexts contain a use of the given declaration. This is relatively expensive.
  KDEVPLATFORMLANGUAGE_EXPORT bool contextHasUse(DUContext* context, Declaration* declaration);

  ///Returns the total count of uses of the given declaration under the given context
  KDEVPLATFORMLANGUAGE_EXPORT uint contextCountUses(DUContext* context, Declaration* declaration);

  ///Returns the declaration that is overridden by the given one, or zero.
  KDEVPLATFORMLANGUAGE_EXPORT Declaration* getOverridden(const Declaration* decl);
  
  ///If the given declaration is a function-declaration, this follows the context-structure up to the function-context that contains the arguments,
  ///and returns it.
  KDEVPLATFORMLANGUAGE_EXPORT DUContext* getFunctionContext(Declaration* decl);
}
}

#endif // KDEVPLATFORM_DUCHAINUTILS_H
