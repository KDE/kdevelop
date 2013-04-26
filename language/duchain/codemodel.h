/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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
      : referenceCount(0)
      , kind(Unknown)
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
    uint referenceCount;
    union {
      Kind kind;
      uint uKind;
    };
    bool operator<(const CodeModelItem& rhs) const {
      return id < rhs.id;
    }
  };

  /**
   * Persistent store that efficiently holds a list of identifiers
   * and their kind for each declaration-string.
   */
  class KDEVPLATFORMLANGUAGE_EXPORT CodeModel
  {
    public:
    CodeModel();
    ~CodeModel();

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

    private:
      class CodeModelPrivate* d;
  };
}

Q_DECLARE_TYPEINFO(KDevelop::CodeModelItem, Q_MOVABLE_TYPE);

#endif
