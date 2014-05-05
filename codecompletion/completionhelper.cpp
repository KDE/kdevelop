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

#include "completionhelper.h"

#include "../duchain/clangtypes.h"
#include "../duchain/cursorkindtraits.h"

#include <language/duchain/stringhelpers.h>
#include <language/editor/simplecursor.h>

#include <QLinkedList>

#include "../util/clangutils.h"

namespace {

struct OverrideInfo
{
    FunctionOverrideList* functions;
    QStringList templateTypes;
    QMap<QString, QString> templateTypeMap;
};

struct ImplementsInfo
{
    CXCursor origin;
    CXCursor top;
    FunctionImplementsList* prototypes;
    QVector<CXCursor> originScope;
    int depth;
    QString templatePrefix;
    QString scopePrefix;
};

constexpr bool canContainFunctionDecls(CXCursorKind kind)
{
    return kind == CXCursor_Namespace || kind == CXCursor_StructDecl ||
           kind == CXCursor_UnionDecl || kind == CXCursor_ClassDecl  ||
           kind == CXCursor_ClassTemplate || kind == CXCursor_ClassTemplatePartialSpecialization;
}

//TODO replace this with clang_Type_getTemplateArgumentAsType when that
//function makes it into the mainstream libclang release.
QStringList templateTypeArguments(CXCursor cursor)
{
    QStringList types;
    QString tStr = ClangString(clang_getTypeSpelling(clang_getCursorType(cursor))).toString();
    ParamIterator iter("<>", tStr);

    while (iter) {
        types.append(*iter);
        ++iter;
    }

    return types;
}

CXChildVisitResult templateParamsHelper(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_TemplateTypeParameter || kind == CXCursor_TemplateTemplateParameter ||
        kind == CXCursor_NonTypeTemplateParameter) {
        (*static_cast<QStringList*>(data)).append(ClangString(clang_getCursorSpelling(cursor)).toString());
    }
    return CXChildVisit_Continue;
}

QStringList templateParams(CXCursor cursor)
{
    QStringList types;
    clang_visitChildren(cursor, templateParamsHelper, &types);
    return types;
}

CXChildVisitResult baseClassVisitor(CXCursor cursor, CXCursor /*parent*/, CXClientData data);

void processBaseClass(CXCursor cursor, FunctionOverrideList* functionList)
{
    QStringList concrete;
    CXCursor ref = clang_getCursorReferenced(cursor);
    CXCursor isTemplate = clang_getSpecializedCursorTemplate(ref);
    if (!clang_Cursor_isNull(isTemplate)) {
        concrete = templateTypeArguments(ref);
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
            for (int i = 0; i < numArgs; i++) {
                CXCursor arg = clang_Cursor_getArgument(cursor, i);
                QString id = ClangString(clang_getCursorDisplayName(arg)).toString();
                QString type = ClangString(clang_getTypeSpelling(clang_getCursorType(arg))).toString();
                if (info->templateTypeMap.contains(type)) {
                    type = info->templateTypeMap.value(type);
                }
                params << type + ' ' + id;
            }

            FuncOverrideInfo fp;
            QString retType = ClangString(clang_getTypeSpelling(clang_getCursorResultType(cursor))).toString();
            if (info->templateTypeMap.contains(retType)) {
                retType = info->templateTypeMap.value(retType);
            }

            fp.returnType = retType;
            fp.name = ClangString(clang_getCursorSpelling(cursor)).toString();
            fp.params =  params;
            fp.isVirtual = clang_CXXMethod_isPureVirtual(cursor);
            fp.isConst = ClangUtils::isConstMethod(cursor);
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
        processBaseClass(cursor, static_cast<FunctionOverrideList*>(data));
    }
    return CXChildVisit_Continue;
}

CXChildVisitResult declVisitor(CXCursor cursor, CXCursor parent, CXClientData d)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    struct ImplementsInfo* data = static_cast<struct ImplementsInfo*>(d);

    //Recurse into cursors which could contain a function declaration
    if (canContainFunctionDecls(kind)) {

        //Don't enter a scope that branches from the origin's scope
        if (data->depth < data->originScope.count() &&
            !clang_equalCursors(cursor, data->originScope.at(data->depth))) {
            return CXChildVisit_Continue;
        }

        QString part, templatePrefix;
        if (data->depth >= data->originScope.count()) {
            QString name = ClangString(clang_getCursorDisplayName(cursor)).toString();

            //This code doesn't play well with anonymous namespaces, so don't recurse
            //into them at all. TODO improve support for anonymous namespaces
            if (kind == CXCursor_Namespace && name.isEmpty()) {
                return CXChildVisit_Continue;
            }

            if (kind == CXCursor_ClassTemplate || kind == CXCursor_ClassTemplatePartialSpecialization) {
                part = name + "::";

                //If we're at a template, we need to construct the template<typename T1, typename T2>
                //which goes at the front of the prototype
                QStringList templateTypes = templateParams(kind == CXCursor_ClassTemplate ? cursor : clang_getSpecializedCursorTemplate(cursor));

                templatePrefix = QString("template<");
                for (int i = 0; i < templateTypes.count(); i++) {
                    templatePrefix = templatePrefix + ((i > 0) ? ", " : "") + "typename " + templateTypes.at(i);
                }
                templatePrefix = templatePrefix + "> ";
            } else {
                part = name + "::";
            }
        }

        ImplementsInfo info{data->origin, data->top, data->prototypes, data->originScope,
                            data->depth + 1,
                            data->templatePrefix + templatePrefix,
                            data->scopePrefix + part};
        clang_visitChildren(cursor, declVisitor, &info);

        return CXChildVisit_Continue;
    }

    if (data->depth < data->originScope.count()) {
        return CXChildVisit_Continue;
    }

    //If the current cursor is not a function or if it is already defined, there's nothing to do here
    if (!CursorKindTraits::isFunction(clang_getCursorKind(cursor)) ||
        !clang_equalCursors(clang_getNullCursor(), clang_getCursorDefinition(cursor)))
    {
        return CXChildVisit_Continue;
    }

    CXCursor origin = data->origin;

    //Don't try to redefine class/structure/union members
    if (clang_equalCursors(origin, parent) && (clang_getCursorKind(origin) != CXCursor_Namespace
                                               && !clang_equalCursors(origin, data->top))) {
        return CXChildVisit_Continue;
    }

    //TODO Add support for pure virtual functions
    QString signature = ClangUtils::getCursorSignature(cursor, data->scopePrefix);

    QString returnType, rest;
    if (kind != CXCursor_Constructor && kind != CXCursor_Destructor) {
        int spaceIndex = signature.indexOf(" ");
        returnType = signature.left(spaceIndex);
        rest = signature.right(signature.count() - spaceIndex - 1);
    } else {
        rest = signature;
    }

    //TODO Add support for pure virtual functions

    data->prototypes->append(FuncImplementInfo{kind == CXCursor_Constructor,
                                               kind == CXCursor_Destructor,
                                               data->templatePrefix, returnType, rest});

    return CXChildVisit_Continue;
}

}

CompletionHelper::CompletionHelper(const CXTranslationUnit& unit, const KDevelop::SimpleCursor& position, const char *file)
{
    CXFile clangFile = clang_getFile(unit, file);
    if (!clangFile) {
        kDebug() << "Completion helper couldn't find file: " << file;
        return;
    }

    CXSourceLocation location = clang_getLocation(unit, clangFile, position.line + 1, position.column + 1);

    if (clang_equalLocations(clang_getNullLocation(), location)) {
        kDebug() << "Completion helper given invalid position " << position
                 << " in file " << file;
        return;
    }

    CXCursor topCursor = clang_getTranslationUnitCursor(unit);
    CXCursor currentCursor = clang_getCursor(unit, location);
    if (clang_getCursorKind(currentCursor) == CXCursor_NoDeclFound) {
        currentCursor = topCursor;
    }

    clang_visitChildren(currentCursor, findBaseVisitor, &m_overrides);

    //TODO This finds functions which aren't yet in scope in the current file
    if (clang_getCursorKind(currentCursor) == CXCursor_Namespace ||
       clang_equalCursors(topCursor,currentCursor)) {

        QLinkedList<CXCursor> scopeList;
        QVector<CXCursor> scopeVec;
        if (!clang_equalCursors(topCursor,currentCursor)) {
            CXCursor search = currentCursor;
            while (!clang_equalCursors(search,topCursor)) {
                scopeList.prepend(clang_getCanonicalCursor(search));
                search = clang_getCursorSemanticParent(search);
            }
            for (auto it = scopeList.begin(); it != scopeList.end(); ++it) {
                scopeVec.append(*it);
            }
        }

        ImplementsInfo info{currentCursor, topCursor, &m_implements, scopeVec, 0, QString(), QString()};
        clang_visitChildren(topCursor, declVisitor, &info);
    }
}

FunctionOverrideList CompletionHelper::overrides() const
{
    return m_overrides;
}

FunctionImplementsList CompletionHelper::implements() const
{
    return m_implements;
}
