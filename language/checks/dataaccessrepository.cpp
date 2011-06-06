/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "dataaccessrepository.h"

namespace KDevelop {

void DataAccessRepository::addModification(const CursorInRevision& cursor, DataAccess::DataAccessFlags flags)
{
    m_modifications.append(new DataAccess(cursor, flags));
}

DataAccess::DataAccess(const CursorInRevision& cur, DataAccess::DataAccessFlags flags)
  : m_flags(flags), m_pos(cur)
{}

DataAccess* DataAccessRepository::accessAt(const CursorInRevision& cursor) const
{
    foreach(DataAccess* a, m_modifications) {
        if(a->pos()==cursor)
            return a;
    }
    return 0;
}

Declaration* DataAccess::declarationForDataAccess() const
{
    Q_ASSERT(false && "TODO!!!!");
    return 0;
}

}

