/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MACROSET_H
#define MACROSET_H

#include <set>
#include "rpp/pp-macro.h"
#include "cppduchainbuilderexport.h"

/**
 * Represents a set of c++ preprocess-macros.
 * */

class QDataStream;
namespace KDevelop {
class HashedString;
};

namespace Cpp {

class KDEVCPPDUCHAINBUILDER_EXPORT MacroSet {
    public:
        typedef std::set< rpp::pp_macro, rpp::pp_macro::NameCompare > Macros;
        MacroSet() : m_idHashValid( false ), m_valueHashValid( false ) {
        }

        void addMacro( const rpp::pp_macro& macro );

        ///@todo reimplement
        void read( QDataStream& stream );

        ///@todo reimplement
        void write( QDataStream& stream ) const;

        bool hasMacro( const QString& name ) const;
        bool hasMacro( const KDevelop::HashedString& name ) const;
        rpp::pp_macro macro( const QString& name ) const;
        
        size_t idHash() const;
        size_t valueHash() const;

        const Macros& macros() const {
          return m_usedMacros;
        }
        
        void merge( const MacroSet& macros );
    private:
        void computeHash() const;
        Macros m_usedMacros;
        mutable bool m_idHashValid;
        mutable bool m_valueHashValid;
        mutable size_t m_idHash; //Hash that represents the ids of all macros
        mutable size_t m_valueHash; //Hash that represents the values of all macros

        friend class Driver;
};

}
#endif
