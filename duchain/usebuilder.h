/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>

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

#include "clangtypes.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>

namespace UseBuilder {

using namespace KDevelop;

inline void createUseCommon(CXCursor cursor, DUContext *parentContext)
{
    auto referenced = clang_getCursorReferenced(cursor);
    auto refLoc = clang_getCursorLocation(referenced);
    CXFile file;
    clang_getFileLocation(refLoc, &file, nullptr, nullptr, nullptr);
    auto url = IndexedString(ClangString(clang_getFileName(file)));
    auto refCursor = CursorInRevision(ClangLocation(refLoc));
    auto useRange = ClangRange(clang_getCursorReferenceNameRange(cursor, CXNameRange_WantSinglePiece, 0)).toRangeInRevision();

    //TODO: handle uses of declarations in other topContexts
    DUChainWriteLocker lock;
    TopDUContext *top = parentContext->topContext();
    if (DUContext *local = top->findContextAt(refCursor)) {
        if (Declaration *used = local->findDeclarationAt(refCursor)) {
            auto usedIndex = top->indexForUsedDeclaration(used);
            parentContext->createUse(usedIndex, useRange);
        }
    }
}

template<CXCursorKind kind> CXChildVisitResult build(CXCursor, DUContext*);

#define AddUseBuilder(CursorKind, Ret)\
template<> CXChildVisitResult build<CursorKind>(CXCursor cursor, DUContext *parentContext)\
{ createUseCommon(cursor, parentContext); return Ret; }

AddUseBuilder(CXCursor_TypeRef, CXChildVisit_Continue)
AddUseBuilder(CXCursor_CXXBaseSpecifier, CXChildVisit_Continue)
AddUseBuilder(CXCursor_TemplateRef, CXChildVisit_Continue)
AddUseBuilder(CXCursor_NamespaceRef, CXChildVisit_Continue)
AddUseBuilder(CXCursor_MemberRef, CXChildVisit_Continue)
AddUseBuilder(CXCursor_LabelRef, CXChildVisit_Continue)
AddUseBuilder(CXCursor_OverloadedDeclRef, CXChildVisit_Continue)
AddUseBuilder(CXCursor_VariableRef, CXChildVisit_Continue)
AddUseBuilder(CXCursor_DeclRefExpr, CXChildVisit_Recurse)
AddUseBuilder(CXCursor_MemberRefExpr, CXChildVisit_Recurse)

}
