/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PERSISTENTSYMBOLTABLE_H
#define KDEVPLATFORM_PERSISTENTSYMBOLTABLE_H

#include <language/languageexport.h>

#include "topducontext.h"

#include <functional>

class QTextStream;

namespace KDevelop {
class IndexedDeclaration;
class IndexedQualifiedIdentifier;

/**
 * Global symbol-table that is stored to disk, and allows retrieving declarations that currently are not loaded to memory.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT PersistentSymbolTable
{
    Q_DISABLE_COPY_MOVE(PersistentSymbolTable)
    PersistentSymbolTable();
public:
    ~PersistentSymbolTable();

    ///Adds declaration @p declaration with id @p id to the symbol table
    ///@warning DUChain must be write locked
    void addDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration);

    ///Adds declaration @p declaration with id @p id to the symbol table
    ///@warning DUChain must be write locked
    void removeDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration);

    enum class VisitorState {
        Break,
        Continue,
    };
    ///@warning The visitor must not call any other PersistentSymbolTable API as that would deadlock
    using DeclarationVisitor = std::function<VisitorState(const IndexedDeclaration&)>;

    /// Iterate over all the declarations for a given IndexedQualifiedIdentifier in an efficient way.
    ///@param id The IndexedQualifiedIdentifier for which the declarations should be retrieved
    ///@param visitor A callback that gets invoked for every matching declaration
    ///@warning DUChain must be read locked
    void visitDeclarations(const IndexedQualifiedIdentifier& id, const DeclarationVisitor& visitor) const;

    /// Iterate over all declarations of the given id, filtered by the visibility given through @a visibility
    /// This is very efficient since it uses a cache
    ///@param id The IndexedQualifiedIdentifier for which the declarations should be retrieved
    ///@param visibility A filter for visibility, only matches in one of these imports will be considered
    ///@param visitor A callback that gets invoked for every matching declaration
    ///@warning DUChain must be read locked
    void visitFilteredDeclarations(const IndexedQualifiedIdentifier& id,
                                   const TopDUContext::IndexedRecursiveImports& visibility,
                                   const DeclarationVisitor& visitor) const;

    static PersistentSymbolTable& self();

    //Very expensive: Checks for problems in the symbol table
    void dump(const QTextStream& out);

    //Clears the internal cache. Should be called regularly to save memory
    void clearCache();
};
}

#endif
