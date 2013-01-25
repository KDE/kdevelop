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

#ifndef QMLJSDUCHAINHELPERS_H_
#define QMLJSDUCHAINHELPERS_H_

#include <language/duchain/declaration.h>

#include "duchainexport.h"

namespace QmlJS
{
using namespace KDevelop;

    /**
     * Find the declaration for a specified node. If no declaration was found,
     * it will return NULL.
     *
     * @param id The qualified identifier that identifies our node.
     * @param range The range for this node.
     * @param context A pointer to the DUContext of this node.
     * @note This method already acquires a read lock for the DUChain.
     */
    KDEVQMLJSDUCHAIN_EXPORT Declaration* getDeclaration(const QualifiedIdentifier &id,
                                                        const RangeInRevision &range,
                                                        DUContextPointer context);

} // End of namespace QmlJS

#endif /* QMLJSDUCHAINHELPERS_H_ */