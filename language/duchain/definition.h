/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef DEFINITION_H
#define DEFINITION_H

#include <editor/documentrangeobject.h>
#include "duchainbase.h"
#include "../languageexport.h"
#include "contextowner.h"

namespace KDevelop
{

class Declaration;
class DUContext;
class TopDUContext;
class DefinitionPrivate;
/**
 * Represents a definition of a previously encountered declaration in a definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT Definition : public DUChainBase, public ContextOwner
{
friend class Declaration;
public:
  Definition(const HashedString& url, const SimpleRange& range, DUContext* context);
  virtual ~Definition();

  virtual TopDUContext* topContext() const;

  DUContext* context() const;
  void setContext(DUContext* context);

  /**
   * Retrieve the declaration for this use.
   * @param topContext The top-context from where this is triggered.
   */
  Declaration* declaration(TopDUContext* topContext = 0) const;

private:
  Q_DECLARE_PRIVATE(Definition)
};
}
#endif // DEFINITION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
