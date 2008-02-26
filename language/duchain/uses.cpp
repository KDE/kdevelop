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

#include "uses.h"
#include "declarationid.h"
#include "duchainpointer.h"
#include <QHash>
#include <QVector>

namespace KDevelop {

struct UsesPrivate {
  QHash<DeclarationId, QList<TopDUContext*> > m_uses;
};

Uses::Uses() : d(new UsesPrivate())
{
}

Uses::~Uses()
{
  delete d;
}

///Assigns @param use to the given @param id.
void Uses::addUse(const DeclarationId& id, TopDUContext* use)
{
    QList<TopDUContext*>& l(d->m_uses[id]);

    if(!l.contains(use))
      l.append(use);
}

void Uses::removeUse(const DeclarationId& id, TopDUContext* use)
{
    QList<TopDUContext*>& l(d->m_uses[id]);

    l.removeAll(use);
}

///Gets the use assigned to @param id, or zero.
QList<TopDUContext*> Uses::uses(const DeclarationId& id) const
{
  QHash<DeclarationId, QList<TopDUContext*> >::const_iterator it = d->m_uses.find(id);
  if(it != d->m_uses.end()) {
    return (*it);
  }else{
    return QList<TopDUContext*>();
  }
}


}

