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

#include "../duchain/cursorkindtraits.h"
#include "../duchain/parsesession.h"
#include "../duchain/documentfinderhelpers.h"
#include "../duchain/clanghelpers.h"
#include "../util/clangdebug.h"
#include "../util/clangtypes.h"
#include "../util/clangutils.h"

#include <language/duchain/stringhelpers.h>

#include <clang-c/Index.h>

#include <algorithm>

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
    QVector<CXFile> fileFilter;
    int depth;
    QString templatePrefix;
};

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

FuncOverrideInfo processCXXMethod(CXCursor cursor, OverrideInfo* info)
{
    QStringList params;

    int numArgs = clang_Cursor_getNumArguments(cursor);
    for (int i = 0; i < numArgs; i++) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);
        QString id = ClangString(clang_getCursorDisplayName(arg)).toString();
        QString type = ClangString(clang_getTypeSpelling(clang_getCursorType(arg))).toString();
        if (info->templateTypeMap.contains(type)) {
            type = info->templateTypeMap.value(type);
        }
        params << type + QLatin1Char(' ') + id;
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
    fp.isConst = clang_CXXMethod_isConst(cursor);

    return fp;
}

CXChildVisitResult baseClassVisitor(CXCursor cursor, CXCursor /*parent*/, CXClientData data);

void processBaseClass(CXCursor cursor, CXCursor parent, FunctionOverrideList* functionList)
{
    QStringList concrete;
    CXCursor ref = clang_getCursorReferenced(cursor);

    if (clang_equalCursors(ref, parent)) {
        return;
    }

    CXCursor isTemplate = clang_getSpecializedCursorTemplate(ref);
    if (!clang_Cursor_isNull(isTemplate)) {
        concrete = ClangUtils::templateArgumentTypes(ref);
        ref = isTemplate;
    }

    OverrideInfo info{functionList, concrete, {}};
    clang_visitChildren(ref, baseClassVisitor, &info);
}

CXChildVisitResult baseClassVisitor(CXCursor cursor, CXCursor parent, CXClientData data)
{
    QString templateParam;
    OverrideInfo* info = static_cast<OverrideInfo*>(data);

    switch(clang_getCursorKind(cursor)) {
    case CXCursor_TemplateTypeParameter:
        templateParam = ClangString(clang_getCursorSpelling(cursor)).toString();
        // TODO: this is probably just a hotfix, find a proper solution to
        //       https://bugs.kde.org/show_bug.cgi?id=355163
        if (info->templateTypes.size() > info->templateTypeMap.size()) {
            info->templateTypeMap.insert(templateParam, info->templateTypes.at(info->templateTypeMap.size()));
        }
        return CXChildVisit_Continue;
    case CXCursor_CXXBaseSpecifier:
        processBaseClass(cursor, parent, info->functions);
        return CXChildVisit_Continue;
    case CXCursor_CXXMethod:
        if (clang_CXXMethod_isVirtual(cursor)) {
            info->functions->append(processCXXMethod(cursor, info));
        }
        return CXChildVisit_Continue;
    default:
        return CXChildVisit_Continue;
    }
}

CXChildVisitResult findBaseVisitor(CXCursor cursor, CXCursor parent, CXClientData data)
{
    auto cursorKind = clang_getCursorKind(cursor);
    if (cursorKind == CXCursor_CXXBaseSpecifier) {
        processBaseClass(cursor, parent, static_cast<FunctionOverrideList*>(data));
    } else if (cursorKind == CXCursor_CXXMethod)   {
        if (!clang_CXXMethod_isVirtual(cursor)) {
            return CXChildVisit_Continue;
        }

        auto info = static_cast<FunctionOverrideList*>(data);

        OverrideInfo overrideInfo {info, {}, {}};
        auto methodInfo = processCXXMethod(cursor, &overrideInfo);
        if (info->contains(methodInfo)) {
            // This method is already implemented, remove it from the list of methods that can be overridden.
            info->remove(info->indexOf(methodInfo), 1);
        }
    }

    return CXChildVisit_Continue;
}

// TODO: make sure we only skip this in classes that actually inherit QObject
bool isQtMocFunction(CXCursor cursor)
{
    static const QByteArray mocFunctions[] = {
        QByteArrayLiteral("metaObject"),
        QByteArrayLiteral("qt_metacast"),
        QByteArrayLiteral("qt_metacall"),
        QByteArrayLiteral("qt_static_metacall"),
    };
    const ClangString function(clang_getCursorSpelling(cursor));
    auto it = std::find(std::begin(mocFunctions), std::end(mocFunctions), function.toByteArray());
    if (it != std::end(mocFunctions)) {
        auto range = ClangRange(clang_getCursorExtent(cursor)).toRange();
        // tokenizing the above range fails for some reason, but
        // if the function comes from a range that happens to be just as wide
        // as the expected Q_OBJECT macro, then we assume this is a moc function
        // and skip it.
        return range.onSingleLine() && range.columnWidth() == strlen("Q_OBJECT");
    }
    return false;
}

CXChildVisitResult declVisitor(CXCursor cursor, CXCursor parent, CXClientData d)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    struct ImplementsInfo* data = static_cast<struct ImplementsInfo*>(d);

    auto location = clang_getCursorLocation(cursor);
    if (clang_Location_isInSystemHeader(location)) {
        // never offer implementation items for system headers
        // TODO: also filter out non-system files unrelated to the current file
        //       e.g. based on the path or similar
        return CXChildVisit_Continue;
    }
    CXFile file = nullptr;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    if (!data->fileFilter.contains(file)) {
        return CXChildVisit_Continue;
    }

    //Recurse into cursors which could contain a function declaration
    if (ClangUtils::isScopeKind(kind)) {

        //Don't enter a scope that branches from the origin's scope
        if (data->depth < data->originScope.count() &&
            !clang_equalCursors(cursor, data->originScope.at(data->depth))) {
            return CXChildVisit_Continue;
        }

        // we must not declare a function outside of its anonymous namespace, so
        // don't recurse into anonymous namespaces if we are not in one already
        if (kind == CXCursor_Namespace && !clang_equalCursors(data->origin, cursor) && ClangString(clang_getCursorDisplayName(cursor)).isEmpty()) {
            return CXChildVisit_Continue;
        }

        QString templatePrefix;
        if (data->depth >= data->originScope.count()) {
            if (kind == CXCursor_ClassTemplate || kind == CXCursor_ClassTemplatePartialSpecialization) {
                //If we're at a template, we need to construct the template<typename T1, typename T2>
                //which goes at the front of the prototype
                const QStringList templateTypes = templateParams(kind == CXCursor_ClassTemplate ? cursor : clang_getSpecializedCursorTemplate(cursor));

                templatePrefix = QLatin1String("template<") + templateTypes.join(QStringLiteral(", typename ")) + QLatin1String("> ");
            }
        }

        ImplementsInfo info{data->origin, data->top, data->prototypes, data->originScope,
                            data->fileFilter,
                            data->depth + 1,
                            data->templatePrefix + templatePrefix};
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

    // don't try to implement pure virtual functions
    if (clang_CXXMethod_isPureVirtual(cursor)) {
        return CXChildVisit_Continue;
    }

    CXCursor origin = data->origin;

    //Don't try to redefine class/structure/union members
    if (clang_equalCursors(origin, parent) && (clang_getCursorKind(origin) != CXCursor_Namespace
                                               && !clang_equalCursors(origin, data->top))) {
        return CXChildVisit_Continue;
    }
    // skip explicitly defaulted/deleted functions as they don't need a definition
    if (ClangUtils::isExplicitlyDefaultedOrDeleted(cursor)) {
        return CXChildVisit_Continue;
    }

    if (isQtMocFunction(cursor) || ClangUtils::specialQtAttributes(cursor) == ClangUtils::QtSignalAttribute) {
        return CXChildVisit_Continue;
    }

    const auto scope = ClangUtils::getScope(cursor, data->origin);
    QString signature = ClangUtils::getCursorSignature(cursor, scope);

    QString returnType, rest;
    if (kind != CXCursor_Constructor && kind != CXCursor_Destructor) {
        int spaceIndex = signature.indexOf(QLatin1Char(' '));
        returnType = signature.left(spaceIndex);
        rest = signature.right(signature.count() - spaceIndex - 1);
    } else {
        rest = signature;
    }

    //TODO Add support for pure virtual functions

    ReferencedTopDUContext top;
    {
        DUChainReadLocker lock;
        top = DUChain::self()->chainForDocument(ClangString(clang_getFileName(file)).toIndexed());
    }
    DeclarationPointer declaration = ClangHelpers::findDeclaration(clang_getCursorLocation(cursor), QualifiedIdentifier(), top);
    data->prototypes->append(FuncImplementInfo{kind == CXCursor_Constructor, kind == CXCursor_Destructor,
                                               data->templatePrefix, returnType, rest, declaration});

    return CXChildVisit_Continue;
}

}

bool FuncOverrideInfo::operator==(const FuncOverrideInfo& rhs) const
{
    return std::make_tuple(returnType, name, params, isConst)
    == std::make_tuple(rhs.returnType, rhs.name, rhs.params, rhs.isConst);
}

CompletionHelper::CompletionHelper()
{
}

void CompletionHelper::computeCompletions(const ParseSession& session, CXFile file, const KTextEditor::Cursor& position)
{
    const auto unit = session.unit();

    CXSourceLocation location = clang_getLocation(unit, file, position.line() + 1, position.column() + 1);

    if (clang_equalLocations(clang_getNullLocation(), location)) {
        clangDebug() << "Completion helper given invalid position " << position
                 << " in file " << clang_getFileName(file);
        return;
    }

    CXCursor topCursor = clang_getTranslationUnitCursor(unit);
    CXCursor currentCursor = clang_getCursor(unit, location);
    if (clang_getCursorKind(currentCursor) == CXCursor_NoDeclFound) {
        currentCursor = topCursor;
    } else if (KTextEditor::Cursor(ClangLocation(clang_getCursorLocation(currentCursor))) >= ClangLocation(location)) {
        currentCursor = clang_getCursorLexicalParent(currentCursor);
    }

    clang_visitChildren(currentCursor, findBaseVisitor, &m_overrides);

    if (clang_getCursorKind(currentCursor) == CXCursor_Namespace ||
       clang_equalCursors(topCursor, currentCursor)) {

        QVector<CXCursor> scopes;
        if (!clang_equalCursors(topCursor, currentCursor)) {
            CXCursor search = currentCursor;
            while (!clang_equalCursors(search, topCursor)) {
                scopes.append(clang_getCanonicalCursor(search));
                search = clang_getCursorSemanticParent(search);
            }
            std::reverse(scopes.begin(), scopes.end());
        }

        QVector<CXFile> fileFilter;
        fileFilter << file;
        const auto& buddies = DocumentFinderHelpers::getPotentialBuddies(QUrl::fromLocalFile(ClangString(clang_getFileName(file)).toString()));
        foreach(const auto& buddy, buddies) {
            auto buddyFile = clang_getFile(unit, qPrintable(buddy.toLocalFile()));
            if (buddyFile) {
                fileFilter << buddyFile;
            }
        }

        ImplementsInfo info{currentCursor, topCursor, &m_implements, scopes, fileFilter, 0, QString()};
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
