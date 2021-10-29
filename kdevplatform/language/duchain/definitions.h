/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DEFINITIONS_H
#define KDEVPLATFORM_DEFINITIONS_H

#include <language/languageexport.h>
#include <util/kdevvarlengtharray.h>

#include <QScopedPointer>

class QTextStream;

namespace KDevelop {
class Declaration;
class IndexedDeclaration;
class DeclarationId;
class TopDUContext;
class DefinitionsPrivate;

/**
 * Global mapping of one Declaration-Ids to multiple Definitions, protected through DUChainLock.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT Definitions
{
public:
    /// Constructor.
    Definitions();
    /// Destructor.
    ~Definitions();
    /**
     * Assigns @param definition to the given @p id
     * */
    void addDefinition(const DeclarationId& id, const IndexedDeclaration& definition);

    void removeDefinition(const DeclarationId& id, const IndexedDeclaration& definition);

    ///Gets all the known definitions assigned to @p id
    KDevVarLengthArray<IndexedDeclaration> definitions(const DeclarationId& id) const;

    /// Dump contents of the definitions repository to stream @p out
    void dump(const QTextStream& out);

private:
    const QScopedPointer<class DefinitionsPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Definitions)
};
}

#endif
