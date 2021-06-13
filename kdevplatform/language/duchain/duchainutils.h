/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_DUCHAINUTILS_H
#define KDEVPLATFORM_DUCHAINUTILS_H

#include <QUrl>

#include <language/languageexport.h>
#include <language/duchain/problem.h>
#include <language/duchain/topducontext.h>

#include <KTextEditor/CodeCompletionModel>

class QIcon;

namespace KTextEditor {
class Cursor;
}

namespace KDevelop {
class Declaration;
class DUChainBase;
class DUContext;
class IndexedString;
class TopDUContext;
class IndexedDeclaration;

/**
 * A namespace which contains convenience utilities for navigating definition-use chains.
 */
namespace DUChainUtils {
KDEVPLATFORMLANGUAGE_EXPORT KTextEditor::CodeCompletionModel::CompletionProperties completionProperties(
    const Declaration* dec);
KDEVPLATFORMLANGUAGE_EXPORT QIcon iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p);
KDEVPLATFORMLANGUAGE_EXPORT QIcon iconForDeclaration(const Declaration* dec);
/** Asks the language-plugins for standard-contexts for the given url, and returns one if available.
 * If there is no language-plugin registered for the given url, it will just try to get any top-context for the file from the du-chain.
 * NOTE: The DUChain needs to be read or write locked when you call this.
 * @param preferProxyContext Whether the returned context should be a proxy context. When no proxy-context is found, a normal context is returned.
 *
 * FIXME: this should operate on IndexedString
 */
KDEVPLATFORMLANGUAGE_EXPORT KDevelop::TopDUContext* standardContextForUrl(const QUrl& url,
                                                                          bool preferProxyContext = false);
/**
 * Returns the content-context associated to the given proxy-context.
 * Returns the same context if it is not a proxy-context.
 * Returns zero if no content-context could be acquired.
 * */
KDEVPLATFORMLANGUAGE_EXPORT TopDUContext* contentContextFromProxyContext(TopDUContext* top);
struct KDEVPLATFORMLANGUAGE_EXPORT ItemUnderCursor
{
    Declaration* declaration; // found declaration (either declared/defined or used)
    DUContext* context; // context in which the declaration, definition, or use was found
    KTextEditor::Range range; // range of the declaration/definition/use
};
/** Returns 1. the Declaration/Definition either declared or used under the cursor,
 * or zero; and 2. the context in which the declaration, definition, or use was found.
 * DUChain must be locked.
 * Must only be called from the foreground or with the foreground lock held. */
KDEVPLATFORMLANGUAGE_EXPORT ItemUnderCursor itemUnderCursor(const QUrl& url, const KTextEditor::Cursor& cursor);
/**If the given declaration is a definition, and has a real declaration
 * attached, returns that declarations. Else returns the given argument. */
KDEVPLATFORMLANGUAGE_EXPORT Declaration* declarationForDefinition(Declaration* definition,
                                                                  TopDUContext* topContext = nullptr);
///Returns the first declaration in the given line. Searches the given context and all sub-contexts.
///Must only be called from the foreground or with the foreground lock held.
KDEVPLATFORMLANGUAGE_EXPORT Declaration* declarationInLine(const KTextEditor::Cursor& cursor, KDevelop::DUContext* ctx);

class KDEVPLATFORMLANGUAGE_EXPORT DUChainItemFilter
{
public:
    virtual bool accept(Declaration* decl) = 0;
    //Should return whether processing should be deepened into the given context
    virtual bool accept(DUContext* ctx) = 0;
    virtual ~DUChainItemFilter();
};
///walks a context, all its sub-contexts, and all its declarations in exactly the order they appear in the file.
///Re-implement DUChainItemFilter to do something with the items.
KDEVPLATFORMLANGUAGE_EXPORT void collectItems(DUContext* context, DUChainItemFilter& filter);

KDEVPLATFORMLANGUAGE_EXPORT DUContext* argumentContext(Declaration* decl);

///Uses the persistent symbol table to find all occurrences of this declaration, based on its identifier.
///The result should be filtered to make sure that the declaration is actually useful to you.
KDEVPLATFORMLANGUAGE_EXPORT QList<IndexedDeclaration> collectAllVersions(Declaration* decl);

///If the given declaration is a class, this gets all classes that inherit this one
///@param collectVersions If this is true, the persistent symbol table is used to first find all registered
///                       versions of this class, and then get the inheriters from them all together. This is needed for C++.
///@param maxAllowedSteps The maximum of steps allowed. If this is zero in the end, this means the search has been stopped with the max. reached
///                                           If you really want _all_ inheriters, you should initialize it with a very large value.
KDEVPLATFORMLANGUAGE_EXPORT QList<Declaration*> inheriters(const Declaration* decl, uint& maxAllowedSteps,
                                                           bool collectVersions = true);

///Gets all functions that override the function @p overriddenDeclaration, starting the search at @p currentClass
///@param maxAllowedSteps The maximum of steps allowed. If this is zero in the end, this means the search has been stopped with the max. reached
KDEVPLATFORMLANGUAGE_EXPORT QList<Declaration*> overriders(const Declaration* currentClass,
                                                           const Declaration* overriddenDeclaration,
                                                           uint& maxAllowedSteps);

///Returns whether the given context or any of its child-contexts contain a use of the given declaration. This is relatively expensive.
KDEVPLATFORMLANGUAGE_EXPORT bool contextHasUse(DUContext* context, Declaration* declaration);

///Returns the total count of uses of the given declaration under the given context
KDEVPLATFORMLANGUAGE_EXPORT uint contextCountUses(DUContext* context, Declaration* declaration);

///Returns the declaration that is overridden by the given one, or zero.
KDEVPLATFORMLANGUAGE_EXPORT Declaration* overridden(const Declaration* decl);

///If the given declaration is a function-declaration, this follows the context-structure up to the function-context that contains the arguments,
///and returns it.
KDEVPLATFORMLANGUAGE_EXPORT DUContext* functionContext(Declaration* decl);

KDEVPLATFORMLANGUAGE_EXPORT QVector<KDevelop::Problem::Ptr> allProblemsForContext(const ReferencedTopDUContext& top);
}
}

#endif // KDEVPLATFORM_DUCHAINUTILS_H
