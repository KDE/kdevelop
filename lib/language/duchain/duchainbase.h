/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "documentrangeobject.h"
#include "kdevlanguageexport.h"

namespace KDevelop
{

class TopDUContext;

/**
 * Base class for definition-use chain objects.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainBase : public KDevelop::DocumentRangeObject
{
public:
  DUChainBase(KTextEditor::Range* range);
  virtual ~DUChainBase();

  int modelRow() const;
  /// TODO atomic set? or assert locked?
  void setModelRow(int row);

  unsigned int lastEncountered() const;
  /// TODO atomic set? or assert locked?
  void setEncountered(unsigned int encountered);

  virtual TopDUContext* topContext() const;

private:
  class DUChainBasePrivate* const d;
};
}

#endif // DUCHAINBASE_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
