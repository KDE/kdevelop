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

extern CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d);

namespace ContextBuilder {

using namespace KDevelop;

inline RangeInRevision makeRange(CXCursor cursor)
{
    auto start = clang_getRangeEnd(clang_Cursor_getSpellingNameRange(cursor, 0, 0));
    auto end = clang_getRangeEnd(clang_getCursorExtent(cursor));
    return RangeInRevision(ClangLocation(start), ClangLocation(end));
}

template<DUContext::ContextType type>
DUContext *createContextCommon(CXCursor cursor, DUContext* parentContext)
{
    auto context = new DUContext(makeRange(cursor), parentContext);
    DUChainWriteLocker lock; //TODO: (..type..) constructor for DUContext?
    context->setType(type);
    clang_visitChildren(cursor, &::visit, context);
    return context;
}

template<DUContext::ContextType type>
DUContext *createContextCommon(CXCursor cursor, const Identifier& id, DUContext* parentContext)
{
    auto context = new DUContext(makeRange(cursor), parentContext);
    DUChainWriteLocker lock; //TODO: (..type, id..) constructor for DUContext?
    context->setType(type);
    context->setLocalScopeIdentifier(parentContext->localScopeIdentifier() + id);
    clang_visitChildren(cursor, &::visit, context);
    return context;
}

template<CXCursorKind kind> DUContext *build(CXCursor, DUContext *) { Q_ASSERT(false); return nullptr; }
#define AddContextBuilder(CursorKind, ContextType)\
template<> DUContext *build<CursorKind>(CXCursor cursor, DUContext *parentContext)\
{ return createContextCommon<ContextType>(cursor, parentContext); }

template<CXCursorKind kind> DUContext *build(CXCursor, const Identifier&, DUContext*) { Q_ASSERT(false); return nullptr; }
#define AddIdContextBuilder(CursorKind, ContextType)\
template<> DUContext *build<CursorKind>(CXCursor cursor, const Identifier& id, DUContext *parentContext)\
{ return createContextCommon<ContextType>(cursor, id, parentContext); }

AddIdContextBuilder(CXCursor_StructDecl, DUContext::Class);
AddIdContextBuilder(CXCursor_UnionDecl, DUContext::Class);
AddIdContextBuilder(CXCursor_ClassDecl, DUContext::Class);
AddIdContextBuilder(CXCursor_EnumDecl, DUContext::Enum);
AddContextBuilder(CXCursor_FunctionDecl, DUContext::Other);
AddContextBuilder(CXCursor_CXXMethod, DUContext::Other);
AddIdContextBuilder(CXCursor_Namespace, DUContext::Namespace);
AddContextBuilder(CXCursor_Constructor, DUContext::Other);
AddContextBuilder(CXCursor_Destructor, DUContext::Other);
AddContextBuilder(CXCursor_ConversionFunction, DUContext::Other);
AddContextBuilder(CXCursor_FunctionTemplate, DUContext::Other);
AddIdContextBuilder(CXCursor_ClassTemplate, DUContext::Class);
AddIdContextBuilder(CXCursor_ClassTemplatePartialSpecialization, DUContext::Class);

}

#endif // CONTEXTBUILDER_H
