/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CODEMODEL_H
#define KDEVPLATFORM_CODEMODEL_H

#include "identifier.h"

namespace KDevelop {
class Declaration;
class IndexedDeclaration;
class DeclarationId;
class TopDUContext;
class QualifiedIdentifier;
class IndexedString;

struct CodeModelItem
{
    CodeModelItem()
        : kind(Unknown)
    {
    }
    enum Kind {
        Unknown = 0,
        Function = 1,
        Variable = 2,
        Class = 4,
        ForwardDeclaration = 8,
        Namespace = 16,
        ClassMember = 32
    };
    IndexedQualifiedIdentifier id;
    uint referenceCount = 0;
    union {
        Kind kind;
        uint uKind;
    };
    bool operator<(const CodeModelItem& rhs) const
    {
        return id < rhs.id;
    }
};

/**
 * Persistent store that efficiently holds a list of identifiers
 * and their kind for each declaration-string.
 */
class KDEVPLATFORMLANGUAGE_EXPORT CodeModel
{
    Q_DISABLE_COPY_MOVE(CodeModel)
public:
    CodeModel();
    /**
     * There can only be one item for each identifier.
     * If an item with this identifier already exists, the kind is updated.
     */
    void addItem(const IndexedString& file, const IndexedQualifiedIdentifier& id, CodeModelItem::Kind kind);

    void removeItem(const IndexedString& file, const IndexedQualifiedIdentifier& id);

    /**
     * Updates the kind for the given item. The item must already be in the code model.
     */
    void updateItem(const IndexedString& file, const IndexedQualifiedIdentifier& id, CodeModelItem::Kind kind);

    /**
     * Retrieves all the global identifiers for a file-name in an efficient way.
     *
     * @param count A reference that will be filled with the count of retrieved items
     * @param items A reference to a pointer, that will represent the array of items.
     *              The array may contain items with an invalid identifier, those should be ignored.
     *              The list is sorted by identifier-index(except for the invalid identifiers in between).
     */
    void items(const IndexedString& file, uint& count, const CodeModelItem*& items) const;

    static CodeModel& self();
};
}

Q_DECLARE_TYPEINFO(KDevelop::CodeModelItem, Q_MOVABLE_TYPE);

#endif
