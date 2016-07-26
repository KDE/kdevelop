/*
    This file is part of KDevelop

    Copyright 2013 Milian Wolff <mail@milianw.de>

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

#include "debugvisitor.h"
#include "util/clangtypes.h"
#include "util/clangutils.h"

namespace {

struct ClientData
{
    QTextStream* out;
    ParseSession* session;
    CXFile file;
    uint depth;
};

CXChildVisitResult visitCursor(CXCursor cursor, CXCursor /*parent*/, CXClientData d)
{
    auto data = static_cast<ClientData*>(d);

    const auto kind = clang_getCursorKind(cursor);

    const auto location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    // don't skip MemberRefExpr with invalid location, see also:
    // http://lists.cs.uiuc.edu/pipermail/cfe-dev/2015-May/043114.html
    if (!ClangUtils::isFileEqual(file, data->file) && (file || kind != CXCursor_MemberRefExpr)) {
        return CXChildVisit_Continue;
    }

    (*data->out) << QByteArray(data->depth * 2, ' ');

    ClangString kindName(clang_getCursorKindSpelling(kind));
    (*data->out) << kindName << " (" << kind << ") ";

    auto type = clang_getCursorType(cursor);
    if (type.kind != CXType_Invalid) {
        ClangString typeName(clang_getTypeSpelling(type));
        (*data->out) << "| type: \"" << typeName << "\"" << " (" << type.kind << ") ";
    }

    auto canonicalType = clang_getCanonicalType(type);
    if (canonicalType.kind != CXType_Invalid
        && !clang_equalTypes(type, canonicalType)) {
        ClangString typeName(clang_getTypeSpelling(canonicalType));
        (*data->out) << "| canonical type: \"" << typeName << "\"" << " (" << canonicalType.kind << ") ";
    }

    auto typedefType = clang_getTypedefDeclUnderlyingType(cursor);
    if (typedefType.kind != CXType_Invalid
        && !clang_equalTypes(type, typedefType)) {
        ClangString typeName(clang_getTypeSpelling(typedefType));
        (*data->out) << "| typedef type: \"" << typeName << "\"" << " (" << typedefType.kind << ") ";
    }

    ClangString displayName(clang_getCursorDisplayName(cursor));
    if (!displayName.isEmpty()) {
        (*data->out) << "| display: \"" << displayName << "\" ";
    }

    auto cursorExtent = ClangRange(clang_getCursorExtent(cursor)).toRange();
    ClangString fileName(clang_getFileName(file));
    (*data->out) << "| loc: " << fileName << '@' << '['
        << '(' << cursorExtent.start().line()+1 << ',' << cursorExtent.start().column()+1 << "),"
        << '(' << cursorExtent.end().line()+1 << ',' << cursorExtent.end().column()+1 << ")] ";

    auto spellingNameRange = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRange();
    (*data->out) << "| sp-name-range: ["
        << '(' << spellingNameRange.start().line()+1 << ',' << spellingNameRange.start().column()+1 << "),"
        << '(' << spellingNameRange.end().line()+1 << ',' << spellingNameRange.end().column()+1 << ")] ";

    if (clang_isDeclaration(kind)) {
        (*data->out) << "| isDecl";
    } else {
        auto referenced = clang_getCursorReferenced(cursor);
        if (kind != CXCursor_UnexposedExpr && !clang_equalCursors(clang_getNullCursor(), referenced)) {
            (*data->out) << "| isUse";
        }
    }

    (*data->out) << endl;

    ClientData childData{data->out, data->session, data->file, data->depth + 1};
    clang_visitChildren(cursor, &visitCursor, &childData);

    return CXChildVisit_Continue;
}

}

DebugVisitor::DebugVisitor(ParseSession* session)
    : m_session(session)
{

}

void DebugVisitor::visit(CXTranslationUnit unit, CXFile file)
{
    auto cursor = clang_getTranslationUnitCursor(unit);
    QTextStream out(stdout);
    ClientData data {&out, m_session, file, 0};
    clang_visitChildren(cursor, &visitCursor, &data);
}
