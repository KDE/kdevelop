/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_LOCALINDEXEDDECLARATION_H
#define KDEVPLATFORM_LOCALINDEXEDDECLARATION_H

#include <language/languageexport.h>
#include <QtGlobal>

namespace KDevelop {
class TopDUContext;
class Declaration;

/**
 * Represents a declaration only by its index within the top-context
 */
class KDEVPLATFORMLANGUAGE_EXPORT LocalIndexedDeclaration
{
public:
    LocalIndexedDeclaration(Declaration* decl = nullptr);
    LocalIndexedDeclaration(uint declarationIndex);

    /**
     * \note Duchain must be read locked
     */
    Declaration* data(TopDUContext* top) const;

    bool operator==(const LocalIndexedDeclaration& rhs) const
    {
        return m_declarationIndex == rhs.m_declarationIndex;
    }
    size_t hash() const
    {
        return m_declarationIndex * 23;
    }

    bool isValid() const
    {
        return m_declarationIndex != 0;
    }

    bool operator<(const LocalIndexedDeclaration& rhs) const
    {
        return m_declarationIndex < rhs.m_declarationIndex;
    }

    /**
     * Index of the Declaration within the top context
     */
    uint localIndex() const
    {
        return m_declarationIndex;
    }

    bool isLoaded(TopDUContext* top) const;

private:
    uint m_declarationIndex;
};
}

Q_DECLARE_TYPEINFO(KDevelop::LocalIndexedDeclaration, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_LOCALINDEXEDDECLARATION_H
