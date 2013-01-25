/*************************************************************************************
 *  Copyright (C) 2013 by Andrea Scarpino <andrea@archlinux.org>                     *
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

DeclarationPointer getDeclaration(const QualifiedIdentifier& id, const RangeInRevision& range, DUContextPointer context)
{
    QList<Declaration *> decls;

    /*
     * Find the declarations at the topContext(). If no declaration was
     * found, we have to look for local declarations. If this fails, we
     * should find for global declarations.
     */
    DUChainReadLocker lock(DUChain::lock());
    if (context.data() == context->topContext())
        decls = context->topContext()->findDeclarations(id, range.end);
    else
        decls = context->topContext()->findDeclarations(id, CursorInRevision::invalid());

    if (decls.isEmpty()) {
        decls = context->findLocalDeclarations(id.last(), range.end);
        if (decls.isEmpty())
            decls = context->findDeclarations(id.last(), range.end);
    }

    if (decls.length()) {
        return DeclarationPointer(decls.last());
    } else {
        return DeclarationPointer();
    }
}

} // End of namespace QmlJS