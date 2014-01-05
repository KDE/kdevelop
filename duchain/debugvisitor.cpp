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

    const auto kind = clang_getCursorKind(cursor);
    if (clang_isDeclaration(kind)) {
        (*data->out) << "decl: ";
    } else {
        auto referenced = clang_getCursorReferenced(cursor);
        if (kind != CXCursor_UnexposedExpr && !clang_equalCursors(clang_getNullCursor(), referenced)) {
            (*data->out) << "use: ";
        } else {
            return CXChildVisit_Recurse;
        }
    }

    ClangString fileName(clang_getFileName(file));
    ClangString displayName(clang_getCursorDisplayName(cursor));
    auto type = clang_getCursorType(cursor);
    ClangString typeName(clang_getTypeSpelling(type));

    (*data->out) << typeName << ' ' << displayName << ' ' << ClangString(clang_getCursorKindSpelling(kind))
                 << " in " << fileName << '@' << line << ':' << column << endl;

    return CXChildVisit_Recurse;
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
    ClientData data {&out, m_session};
    clang_visitChildren(cursor, &::visit, &data);
}
