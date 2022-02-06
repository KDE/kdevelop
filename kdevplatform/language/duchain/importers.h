/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_IMPORTERS_H
#define KDEVPLATFORM_IMPORTERS_H

#include <language/languageexport.h>
#include "declarationid.h"

namespace KDevelop {
class DeclarationId;
class IndexedDUContext;

/**
 * Global mapping of Declaration-Ids to contexts that import the associated context, protected through DUChainLock.
 * This is used as an alternative to the local importers list within DUContext, only for indirect imports(Across different files, or with templates).
 *
 * This has the advantage that imports stay valid even if the imported context is deleted temporarily, and stored top-contexts don't need to be
 * changed just because imports were added.
 *
 * To retrieve the actual uses, query the duchain for the files.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT Importers
{
    Q_DISABLE_COPY_MOVE(Importers)
public:
    Importers();
    /**
     * Adds a top-context to the users-list of the given id
     * */
    void addImporter(const DeclarationId& id, const IndexedDUContext& use);
    /**
     * Removes the given top-context from the list of uses
     * */
    void removeImporter(const DeclarationId& id, const IndexedDUContext& use);

    ///Gets the top-contexts of all users assigned to the declaration-id
    KDevVarLengthArray<IndexedDUContext> importers(const DeclarationId& id) const;

    static Importers& self();
};
}

#endif
