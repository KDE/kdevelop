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

#ifndef KDEVPLATFORM_USES_H
#define KDEVPLATFORM_USES_H

#include "../languageexport.h"
#include <util/kdevvarlengtharray.h>

namespace KDevelop {

  class DeclarationId;
  class IndexedTopDUContext;

/**
 * Global mapping of Declaration-Ids to top-contexts, protected through DUChainLock.
 *
 * To retrieve the actual uses, query the duchain for the files.
 * */
  class KDEVPLATFORMLANGUAGE_EXPORT Uses {
    public:
    /// Constructor.
    Uses();
    /// Destructor.
    ~Uses();
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

    private:
      class UsesPrivate* d;
  };
}

#endif
