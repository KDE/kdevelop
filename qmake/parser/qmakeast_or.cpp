/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakeast.h"
#include <kdebug.h>

namespace QMake
{
    OrAST::OrAST( AST* parent )
        : ScopeAST( parent ), m_lCall( 0 ), m_rCall( 0 )
    {}

    OrAST::~OrAST()
    {
        delete m_lCall;
        m_lCall = 0;
        delete m_rCall;
        m_rCall = 0;
    }

    void OrAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        m_lCall->writeToString( buf );
        buf += m_orop;
        m_rCall->writeToString( buf );
        ScopeAST::writeToString( buf );
    }

    FunctionCallAST* OrAST::leftCall() const
    {
        return m_lCall;
    }

    FunctionCallAST* OrAST::rightCall() const
    {
        return m_rCall;
    }

    void OrAST::setLeftCall( FunctionCallAST* call )
    {
        m_lCall = call;
    }

    void OrAST::setRightCall( FunctionCallAST* call )
    {
        m_rCall = call;
    }

    QString OrAST::orOp() const
    {
        return m_orop;
    }

    void OrAST::setOrOp( const QString& orop )
    {
        m_orop = orop;
    }
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
