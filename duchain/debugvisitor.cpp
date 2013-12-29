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

namespace {

struct ClientData
{
    QTextStream* out;
};

struct ClangString
{
    ClangString(CXString string)
        : string(string)
    {
    }

    ~ClangString()
    {
        clang_disposeString(string);
    }

    operator const char*() const
    {
        return clang_getCString(string);
    }

    CXString string;
};

CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData d)
{
    auto data = static_cast<ClientData*>(d);
    const auto kind = clang_getCursorKind(cursor);
    if (clang_isDeclaration(kind)) {
        ClangString displayName = clang_getCursorDisplayName(cursor);
        auto type = clang_getCursorType(cursor);
        ClangString typeName = clang_getTypeSpelling(type);

        auto location = clang_getCursorLocation(cursor);
        CXFile file;
        uint line;
        uint column;
        clang_getFileLocation(location, &file, &line, &column, 0);
        ClangString fileName = clang_getFileName(file);
        (*data->out) << typeName << ' ' << displayName << " in " << fileName << '@' << line << ':' << column << endl;
    }

    return CXChildVisit_Recurse;
}

}

DebugVisitor::DebugVisitor(ParseSession* session)
    : m_session(session)
{

}

DebugVisitor::~DebugVisitor()
{

}

void DebugVisitor::startVisiting(CXTranslationUnit unit)
{
    auto cursor = clang_getTranslationUnitCursor(unit);
    QTextStream out(stdout);
    ClientData data {&out};
    clang_visitChildren(cursor, visit, &data);
}
