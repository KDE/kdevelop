/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_USES_H
#define KDEVPLATFORM_USES_H

#include <language/languageexport.h>
#include <util/kdevvarlengtharray.h>

namespace KDevelop {
class DeclarationId;
class IndexedTopDUContext;

/**
 * Global mapping of Declaration-Ids to top-contexts, protected through DUChainLock.
 *
 * To retrieve the actual uses, query the duchain for the files.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT Uses
{
    Q_DISABLE_COPY_MOVE(Uses)
public:
    Uses();
    /**
     * Adds a top-context to the users-list of the given id
     * */
    void addUse(const DeclarationId& id, const IndexedTopDUContext& use);
    /**
     * Removes the given top-context from the list of uses
     * */
    void removeUse(const DeclarationId& id, const IndexedTopDUContext& use);
    /**
     * Checks whether the given DeclarationID is is used
     * */
    bool hasUses(const DeclarationId& id) const;

    ///Gets the top-contexts of all users assigned to the declaration-id
    KDevVarLengthArray<IndexedTopDUContext> uses(const DeclarationId& id) const;
};
}

#endif
