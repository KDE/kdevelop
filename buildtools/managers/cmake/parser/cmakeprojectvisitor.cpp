/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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
#include "cmakeprojectvisitor.h"
#include "cmakeast.h"

#include <kdebug.h>


void CMakeProjectVisitor::notImplemented() const {
    kDebug(9032) << "not implemented!" << endl;
}

void CMakeProjectVisitor::visit(const CMakeAst *ast)
{
    QList<CMakeAst*> children = ast->children();
    QList<CMakeAst*>::const_iterator it = children.begin();
    QList<CMakeAst*>::const_iterator end = children.end();
    for(; it!=end; it++) {
        kDebug(9032) << "Parsing ast" << endl;
        ast->accept(this);
    }
}
