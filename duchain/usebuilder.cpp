/*
    This file is part of KDevelop

    Copyright 2014 Olivier de Gaalon <olivier.jg@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "usebuilder.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>

using namespace KDevelop;

namespace UseBuilder {

void createUseCommon(CXCursor cursor, DUContext *parentContext, const IncludeFileContexts& includes)
{
    auto referenced = clang_getCursorReferenced(cursor);
    auto used = findDeclaration(referenced, includes);
    if (!used) {
        return;
    }
    auto useRange = ClangRange(clang_getCursorReferenceNameRange(cursor, CXNameRange_WantSinglePiece, 0)).toRangeInRevision();
    DUChainWriteLocker lock;
    if (used) {
        auto usedIndex = parentContext->topContext()->indexForUsedDeclaration(used.data());
        parentContext->createUse(usedIndex, useRange);
    }
}

}
