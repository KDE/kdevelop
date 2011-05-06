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

#ifndef MODIFICATIONSREPOSITORY_H
#define MODIFICATIONSREPOSITORY_H
#include <language/editor/simplecursor.h>
#include <cppduchainexport.h>

namespace KDevelop
{

class Declaration;

//DataAccess
class DataAccess
{
    public:
        enum Flags { None=0, Read=1, Write=2 };
        DataAccess(const SimpleCursor& cur, uint flags);
        
        uint m_flags;
        KDevelop::SimpleCursor m_pos;
        //SimpleRange value() const; //nomes per write?
        
        Declaration* declarationForDataAccess() const;
};


class KDEVCPPDUCHAIN_EXPORT DataAccessRepository
{
    public:
        //Again, cursor/range?
        void addModification(const KDevelop::SimpleCursor& cursor, uint flags);
      
        void clear() { m_modifications.clear(); }
        int count() const { return m_modifications.size(); }
        const DataAccess& at(int i) const { return m_modifications.at(i); }
    private:
        QList<DataAccess> m_modifications;
};

}
#endif
