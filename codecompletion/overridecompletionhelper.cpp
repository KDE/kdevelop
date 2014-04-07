/*
 * This file is part of KDevelop
 * Copyright 2014 David Stevens <dgedstevens@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "overridecompletionhelper.h"

#include "../duchain/clangtypes.h"

struct OverrideInfo
{
    FunctionInfoList* functions;
    QStringList templateTypes;
    QMap<QString, QString> templateTypeMap;
};

//TODO replace this with clang_Type_getTemplateArgumentAsType when that
//function makes it into the mainstream libclang release.
void getTemplateTypes(CXCursor cursor, QStringList& types)
{
    QString tStr = ClangString(clang_getTypeSpelling(clang_getCursorType(cursor))).toString();
    int depth = 0;
    int start = -1;
    int cur = 0;
    int length = tStr.size();
    while (cur < length) {
        char c = tStr.at(cur).toAscii();
        if (c == '<') {
            depth++;
            if (depth == 1) {
                start = cur+1;
            }
        } else if (c == '>') {
            depth--;
            if (depth == 0) {
                types.append(tStr.mid(start, cur - start).trimmed());
            }
        } else if (c == ',') {
            if (depth == 1) {
                types.append(tStr.mid(start, cur - start).trimmed());
                start = cur + 1;
            }
        }
        cur++;
    }
}

CXChildVisitResult baseClassVisitor(CXCursor cursor, CXCursor /*parent*/, CXClientData data);

void processBaseClass(CXCursor cursor, FunctionInfoList* functionList)
{
    QStringList concrete;
    CXCursor ref = clang_getCursorReferenced(cursor);
    CXCursor isTemplate = clang_getSpecializedCursorTemplate(ref);
    if (!clang_Cursor_isNull(isTemplate)) {
        getTemplateTypes(ref, concrete);
        ref = isTemplate;
    }

    OverrideInfo info{functionList, concrete, {}};
    clang_visitChildren(ref, baseClassVisitor, &info);
}

CXChildVisitResult baseClassVisitor(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
    QString templateParam;
    OverrideInfo* info = static_cast<OverrideInfo*>(data);

    switch(clang_getCursorKind(cursor)) {
    case CXCursor_TemplateTypeParameter:
        templateParam = ClangString(clang_getCursorSpelling(cursor)).toString();
        info->templateTypeMap.insert(templateParam, info->templateTypes.at(info->templateTypeMap.size()));
        return CXChildVisit_Continue;
    case CXCursor_CXXBaseSpecifier:
        processBaseClass(cursor, info->functions);
        return CXChildVisit_Continue;
    case CXCursor_CXXMethod:

        if (clang_CXXMethod_isVirtual(cursor)) {
            QStringList params;

            int numArgs = clang_Cursor_getNumArguments(cursor);
            for (int i = 0; i < numArgs; i++)
            {
                CXCursor arg = clang_Cursor_getArgument(cursor, i);
                QString id = ClangString(clang_getCursorDisplayName(arg)).toString();
                QString type = ClangString(clang_getTypeSpelling(clang_getCursorType(arg))).toString();
                if (info->templateTypeMap.contains(type)) {
                    type = info->templateTypeMap.value(type);
                }
                params << type + ' ' + id;
            }

            FunctionInfo fp;
            QString retType = ClangString(clang_getTypeSpelling(clang_getCursorResultType(cursor))).toString();
            if (info->templateTypeMap.contains(retType)) {
                retType = info->templateTypeMap.value(retType);
            }

            fp.returnType = retType;
            fp.name = ClangString(clang_getCursorSpelling(cursor)).toString();
            fp.params =  params;
            fp.isVirtual = clang_CXXMethod_isPureVirtual(cursor);

            //FIXME The clang-c API currently doesn't provide access to a function declaration's
            //const qualifier. This parses the Unified Symbol Resolution to retrieve that information.
            //However, since the USR is undocumented, this might break in the future.
            QString usr = ClangString(clang_getCursorUSR(cursor)).toString();
            if (usr.at(usr.length() - 2) == '#') {
                fp.isConst = ((usr.at(usr.length() - 1).toAscii()) - '0') & 0x1;
            } else {
                fp.isConst = false;
            }

            info->functions->append(fp);
        }
        return CXChildVisit_Continue;
    default:
        return CXChildVisit_Continue;
    }
}

CXChildVisitResult findBaseVisitor(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
    if (clang_getCursorKind(cursor) == CXCursor_CXXBaseSpecifier) {
        processBaseClass(cursor, static_cast<FunctionInfoList*>(data));
    }
    return CXChildVisit_Continue;
}

OverrideCompletionHelper::OverrideCompletionHelper(const CXTranslationUnit& unit, const KDevelop::SimpleCursor& position, const char *file)
{
    CXFile clangFile = clang_getFile(unit, file);
    if (!clangFile) {
        kDebug() << "Override completion helper couldn't find file.";
        return;
    }

    CXSourceLocation location = clang_getLocation(unit, clangFile, position.line + 1, position.column + 1);

    if (clang_equalLocations(clang_getNullLocation(), location)) {
        kDebug() << "Override completion helper given invalid position: " << file;
        return;
    }

    CXCursor cursor = clang_getCursor(unit, location);
    clang_visitChildren(cursor, findBaseVisitor, &m_overrides);
}

FunctionInfoList OverrideCompletionHelper::getOverrides()
{
    return m_overrides;
}
