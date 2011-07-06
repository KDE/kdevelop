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

/**
 * @brief Represents a data access in some code
 * 
 * This class provides the position of a data access in the code
 * and tells us whether it's writing or reading the data.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DataAccess
{
    public:
        /** Defines the flags that will tell what this data access is about */
        enum DataAccessFlag { None=0,
            Read=1, /**< This data access reads data */
            Write=2 /**< This data access writes data */
        };
        Q_DECLARE_FLAGS(DataAccessFlags, DataAccessFlag)
        
        /** Constructs a DataAccess instance with its @p cur position and
         * its @p flags DataAccessFlags that will tell us how is it modifying the data
         */
        DataAccess(const CursorInRevision& cur, DataAccessFlags flags);
        
        /** Checks the flags and returns if it's reading the data */
        bool isRead()  const { return m_flags&Read; }
        
        /** Checks the flags and returns if it's writing the data */
        bool isWrite() const { return m_flags&Write; }
        
        /** @returns the cursor */
        KDevelop::CursorInRevision pos() const { return m_pos; }
        
        /** @returns the flags that specify how is this access interacting with the data */
        DataAccessFlags flags() const { return m_flags; }
        
    private:
        DataAccessFlags m_flags;
        KDevelop::CursorInRevision m_pos;
};

/**
 * @brief Stores all the data accesses in a file
 * 
 * Provides the data accesses in a file and provides different ways to accessing them
 */
class KDEVPLATFORMLANGUAGE_EXPORT DataAccessRepository
{
    public:
        ~DataAccessRepository() { clear(); }
        
        /** Constructs a DataAccess instance and adds it to the repository */
        void addModification(const KDevelop::CursorInRevision& cursor, DataAccess::DataAccessFlags flags);
      
        /** Clears the whole structure as if it was never used before */
        void clear() { qDeleteAll(m_modifications); m_modifications.clear(); }
        
        /** @returns all the data access stored in this repository */
        QList<DataAccess*> modifications() const { return m_modifications; }
        
        /** @returns the access located at the position specified by @p cursor */
        DataAccess* accessAt(const KDevelop::CursorInRevision& cursor) const;
    private:
        QList<DataAccess*> m_modifications;
};

}
#endif
