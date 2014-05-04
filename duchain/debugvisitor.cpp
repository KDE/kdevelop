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
#include "clangtypes.h"

namespace {

struct ClientData
{
    QTextStream* out;
    ParseSession* session;
    uint depth;
};

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d)
{
    auto data = static_cast<ClientData*>(d);

    auto location = clang_getCursorLocation(cursor);
    CXFile file;
    uint line;
    uint column;
    clang_getFileLocation(location, &file, &line, &column, 0);
    if (file != data->session->file()) {
        return CXChildVisit_Continue;
    }

    (*data->out) << QByteArray(data->depth * 2, ' ');

    const auto kind = clang_getCursorKind(cursor);
    if (clang_isDeclaration(kind)) {
        (*data->out) << "decl: ";
    } else {
        auto referenced = clang_getCursorReferenced(cursor);
        if (kind != CXCursor_UnexposedExpr && !clang_equalCursors(clang_getNullCursor(), referenced)) {
            (*data->out) << "use: ";
        }
    }

    (*data->out) << '"';

    auto type = clang_getCursorType(cursor);
    if (type.kind != CXType_Invalid) {
        ClangString typeName(clang_getTypeSpelling(type));
        (*data->out) << typeName << ' ';
    }

    ClangString displayName(clang_getCursorDisplayName(cursor));
    if (strlen(displayName)) {
        (*data->out) << displayName << ' ';
    }

    (*data->out) << '"';

    ClangString kindName(clang_getCursorKindSpelling(kind));
    (*data->out) << " of kind "  << kindName << " (" << kind << ")";

    ClangRange range(clang_getCursorExtent(cursor));
    KDevelop::SimpleRange simpleRange = range.toSimpleRange();
    ClangString fileName(clang_getFileName(file));
    (*data->out) << " in " << fileName << '@' << '['
        << '(' << simpleRange.start.line+1 << ',' << simpleRange.start.column+1 << "),"
        << '(' << simpleRange.end.line+1 << ',' << simpleRange.end.column+1 << ")]";

    (*data->out) << endl;

    ClientData childData{data->out, data->session, data->depth + 1};
    clang_visitChildren(cursor, &::visit, &childData);

    return CXChildVisit_Continue;
}

}

DebugVisitor::DebugVisitor(ParseSession* session)
    : m_session(session)
{

}

void DebugVisitor::visit(CXTranslationUnit unit)
{
    auto cursor = clang_getTranslationUnitCursor(unit);
    QTextStream out(stdout);
    ClientData data {&out, m_session, 0};
    clang_visitChildren(cursor, &::visit, &data);
}
