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
#include <language/languageexport.h>
#include <language/editor/cursorinrevision.h>

namespace KDevelop
{

class Declaration;

//DataAccess
class KDEVPLATFORMLANGUAGE_EXPORT DataAccess
{
    public:
        enum Flags { None=0, Read=1, Write=2 };
        DataAccess(const CursorInRevision& cur, uint flags);
        
        bool isRead()  const { return m_flags&Read; }
        bool isWrite() const { return m_flags&Write; }
        
        uint m_flags;
        KDevelop::CursorInRevision m_pos;
        //SimpleRange value() const; //nomes per write?
        
        Declaration* declarationForDataAccess() const;
};


class KDEVPLATFORMLANGUAGE_EXPORT DataAccessRepository
{
    public:
        ~DataAccessRepository() { clear(); }
        //Again, cursor/range?
        void addModification(const KDevelop::CursorInRevision& cursor, uint flags);
      
        void clear() { qDeleteAll(m_modifications); m_modifications.clear(); }
        QList<DataAccess*> modifications() const { return m_modifications; }
        DataAccess* accessAt(const KDevelop::CursorInRevision& cursor) const;
    private:
        QList<DataAccess*> m_modifications;
};

}
#endif
