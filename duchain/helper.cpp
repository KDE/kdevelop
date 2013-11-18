/*************************************************************************************
 *  Copyright (C) 2013 by Andrea Scarpino <scarpino@kde.org>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "helper.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

namespace QmlJS
{
using namespace KDevelop;

DeclarationPointer getDeclaration(const QualifiedIdentifier& id, const DUContextPointer& context)
{
    DUChainReadLocker lock;
    if (context) {
        foreach (Declaration* dec, context->findDeclarations(id)) {
            return DeclarationPointer(dec);
        }
    }
    return DeclarationPointer();
}

} // End of namespace QmlJS
