/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DEFINITIONUSE_H
#define KDEVPLATFORM_DEFINITIONUSE_H

#include <limits>

#include "../editor/rangeinrevision.h"

#include <language/languageexport.h>

namespace KDevelop {
class TopDUContext;
class Declaration;
/**
 * Represents a position in a document where a specific declaration is used.
 *
 * Since we want to build uses for all files, and every single function may contain
 * tens to hundreds of uses, uses must be extremely light-weight.
 *
 * For that reason they are built in a way that a use can completely be stored in a simple vector,
 * and they only contain indices that represent the actual declaration used. Since the same
 * Declarations are used over and over again, the actual declarations are stored and indexed centrally
 * in the enclosing top-context. Additionally, because a use may refer to a not globally addressable item,
 * each top-context contains a local map that maps declaration-indices to local declarations.
 *
 * Since only a small fraction of all files is loaded as document at any time, only few documents actually
 * need smart-ranges. For that reason we do not store them here, but instead only map them to the uses
 * when there actually IS smart-ranges for them.
 */
class KDEVPLATFORMLANGUAGE_EXPORT Use
{
public:

    explicit Use(const RangeInRevision& range = RangeInRevision::invalid(),
                 int declarationIndex = std::numeric_limits<int>::max())
        : m_range(range)
        , m_declarationIndex(declarationIndex)
    {
    }

    Declaration* usedDeclaration(TopDUContext* topContext) const;

    RangeInRevision m_range;
    int m_declarationIndex;
};
}

Q_DECLARE_TYPEINFO(KDevelop::Use, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_DEFINITIONUSE_H
