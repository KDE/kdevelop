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

#ifndef DUCHAINBASE_H
#define DUCHAINBASE_H

#include <editor/documentrangeobject.h>
#include <editor/hashedstring.h>
#include "../languageexport.h"

namespace KDevelop
{

class TopDUContext;
class DUChainBase;
class DUChainPointerData;


/**
 * Base class for definition-use chain objects.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainBase : public KDevelop::DocumentRangeObject
{
public:
  DUChainBase(const HashedString& url, const SimpleRange& range);
  virtual ~DUChainBase();

  virtual TopDUContext* topContext() const;

  /**
   * Returns a special pointer that can be used to track the existence of a du-chain object across locking-cycles.
   * @see DUChainPointerData
   * */
  const KSharedPtr<DUChainPointerData>& weakPointer() const;

protected:
  DUChainBase( class DUChainBasePrivate& dd );
  DUChainBase( class DUChainBasePrivate& dd, const HashedString& url, const SimpleRange& range );
  
private:
  Q_DECLARE_PRIVATE(DUChainBase)
  mutable KSharedPtr<DUChainPointerData> m_ptr;
};
}

#endif // DUCHAINBASE_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
