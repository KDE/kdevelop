/*
 *    This file is part of KDevelop
 *
 *    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#include <unordered_map>

#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/delayedtype.h>

#include "buildduchainvisitor.h"
#include "clangtypes.h"

using namespace KDevelop;

namespace {
    bool isContextKind(CXCursorKind kind)
    {
        switch (kind)
        {
        case CXCursor_ClassDecl:
        case CXCursor_Namespace:
        case CXCursor_FunctionDecl:
        case CXCursor_StructDecl:
        case CXCursor_CXXMethod:
        case CXCursor_ClassTemplatePartialSpecialization:
            return true;
        default:
            return false;
        }
    }
    bool isSkipIntoKind(CXCursorKind kind)
    {
        switch (kind) {
            case CXCursor_CompoundStmt:
            case CXCursor_DeclStmt:
                return true;
            default:
                return false;
        }
    }

    DUContext *buildContextForCursor(CXCursor cursor, KDevelop::DUContext *parentContext)
    {
        if (!isContextKind(clang_getCursorKind(cursor)))
            return nullptr;

        DUChainWriteLocker lock;
        auto range = ClangRange{clang_getCursorExtent(cursor)};
        return new DUContext(range.toRangeInRevision(), parentContext);
    }

    AbstractType::Ptr buildTypeForCursor(CXCursor cursor)
    {
        auto type = clang_getCursorType(cursor);
        auto identifier = IndexedTypeIdentifier(QString(ClangString(clang_getTypeSpelling(type))));

        DelayedType *delayedType = new DelayedType;
        delayedType->setIdentifier(identifier);
        delayedType->setKind(DelayedType::Unresolved);
        return AbstractType::Ptr(delayedType);
    }

    Declaration *buildDeclarationForCursor(CXCursor cursor, KDevelop::DUContext *parentContext, KDevelop::DUContext *internalContext)
    {
        if (!clang_isDeclaration(clang_getCursorKind(cursor)))
            return nullptr;
        //TODO: figure out how to use pieceIndex
        auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRangeInRevision();
        auto identifier = Identifier(IndexedString(ClangString(clang_getCursorSpelling(cursor))));
        auto comment = QByteArray{ClangString(clang_Cursor_getRawCommentText(cursor))};

        AbstractType::Ptr type = buildTypeForCursor(cursor);

        DUChainWriteLocker lock;
        auto decl = new KDevelop::Declaration(range, parentContext);
        decl->setComment(comment);
        decl->setIdentifier(identifier);
        decl->setInternalContext(internalContext);
        decl->setAbstractType(type);
        return decl;
    }

    CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d)
    {
        if (isSkipIntoKind(clang_getCursorKind(cursor)))
            return CXChildVisit_Recurse;

        auto parentContext = static_cast<DUContext*>(d);

        auto context = buildContextForCursor(cursor, parentContext);
        if (context)
        {
            clang_visitChildren(cursor, &::visit, context);
        }

        buildDeclarationForCursor(cursor, parentContext, context);

        return CXChildVisit_Continue;
    }
}

void BuildDUChainVisitor::visit(CXTranslationUnit unit, ReferencedTopDUContext top)
{
    auto cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, &::visit, top.data());
}
