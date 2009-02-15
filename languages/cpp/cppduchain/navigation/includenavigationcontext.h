/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef INCLUDENAVIGATIONCONTEXT_H
#define INCLUDENAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include "../../includeitem.h"

namespace Cpp {
  
class IncludeNavigationContext : public KDevelop::AbstractNavigationContext {
public:
  IncludeNavigationContext(const IncludeItem& item, KDevelop::TopDUContextPointer topContext);
  virtual QString html(bool shorten);
  virtual QString name() const;

private:
  ///@param first must initially be true
  void addDeclarationsFromContext(KDevelop::DUContext* ctx, bool& first, QString indent = "");
  IncludeItem m_item;
};

}

#endif
