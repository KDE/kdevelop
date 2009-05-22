/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

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

#include "utilities.h"

#include <duchain/indexedstring.h>
#include <duchain/identifier.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>

namespace KDevelop
{

namespace CodeGenUtils
{

QValidator::State IdentifierValidator::validate (QString & input, int &) const
{
    //I can't figure out why it wouln't compile when I tried to use Identifier identifier();
    Identifier identifier = Identifier(IndexedString(input));
    
    if(identifier.isUnique())
        return Acceptable;
    
    DUChainReadLocker lock(DUChain::lock(), 10);
    return m_context->allLocalDeclarations(identifier).empty() ? Acceptable : Invalid;
}

}


}
