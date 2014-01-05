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

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include "clangtypes.h"

#include <language/duchain/duchainlock.h>

namespace ContextBuilder {

using namespace KDevelop;

inline RangeInRevision makeRange(CXCursor cursor)
{
    auto start = clang_getRangeEnd(clang_Cursor_getSpellingNameRange(cursor, 0, 0));
    auto end = clang_getRangeEnd(clang_getCursorExtent(cursor));
    return {ClangLocation(start), ClangLocation(end)};
}

template<DUContext::ContextType type>
DUContext *createContextCommon(CXCursor cursor, DUContext* parentContext)
{
    auto context = new DUContext(makeRange(cursor), parentContext);
    DUChainWriteLocker lock; //TODO: (..type..) constructor for DUContext?
    context->setType(type);
    return context;
}

constexpr DUContext::ContextType contextType(CXCursorKind CK)
{
    return CK == CXCursor_StructDecl                         ? DUContext::Class
         : CK == CXCursor_UnionDecl                          ? DUContext::Class
         : CK == CXCursor_StructDecl                         ? DUContext::Class
         : CK == CXCursor_UnionDecl                          ? DUContext::Class
         : CK == CXCursor_ClassDecl                          ? DUContext::Class
         : CK == CXCursor_EnumDecl                           ? DUContext::Enum
         : CK == CXCursor_FunctionDecl                       ? DUContext::Other
         : CK == CXCursor_CXXMethod                          ? DUContext::Other
         : CK == CXCursor_Namespace                          ? DUContext::Namespace
         : CK == CXCursor_Constructor                        ? DUContext::Other
         : CK == CXCursor_Destructor                         ? DUContext::Other
         : CK == CXCursor_ConversionFunction                 ? DUContext::Other
         : CK == CXCursor_FunctionTemplate                   ? DUContext::Other
         : CK == CXCursor_ClassTemplate                      ? DUContext::Class
         : CK == CXCursor_ClassTemplatePartialSpecialization ? DUContext::Class
         : static_cast<DUContext::ContextType>(-1);
}

template<CXCursorKind CK, class Enable = typename std::enable_if<contextType(CK) != -1>::type>
DUContext *build(CXCursor cursor, const Identifier& id, DUContext* parentContext)
{
    auto context = new DUContext(makeRange(cursor), parentContext);
    DUChainWriteLocker lock; //TODO: (..type, id..) constructor for DUContext?
    context->setType(contextType(CK));
    context->setLocalScopeIdentifier(parentContext->localScopeIdentifier() + id);
    return context;
}

}

#endif // CONTEXTBUILDER_H
