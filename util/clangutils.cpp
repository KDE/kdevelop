/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
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
 *
 */

#include "clangutils.h"

#include "../duchain/clangtypes.h"
#include "../duchain/cursorkindtraits.h"

#include <language/duchain/indexedstring.h>

#include <clang-c/Index.h>
#include <kdebug.h>

#include <QDir>
#include <QProcess>
#include <QVector>

using namespace KDevelop;

CXCursor ClangUtils::getCXCursor(int line, int column, const CXTranslationUnit& unit, const CXFile& file)
{
    if (!file) {
        kDebug() << "getCXCursor couldn't find file: " << ClangString(clang_getFileName(file));
        return clang_getNullCursor();
    }

    CXSourceLocation location = clang_getLocation(unit, file, line + 1, column + 1);

    if (clang_equalLocations(clang_getNullLocation(), location)) {
        kDebug() << "getCXCursor given invalid position " << line << ", " << column
                 << " for file " << ClangString(clang_getFileName(file));
        return clang_getNullCursor();
    }

    return clang_getCursor(unit, location);
}

namespace {

struct FunctionInfo {
    SimpleRange range;
    QString fileName;
    CXTranslationUnit unit;
    QStringList stringParts;
};

CXChildVisitResult paramVisitor(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
    //Ignore the type of the parameter
    CXCursorKind kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_TypeRef || kind == CXCursor_TemplateRef || kind == CXCursor_NamespaceRef) {
        return CXChildVisit_Continue;
    }

    FunctionInfo *info = static_cast<FunctionInfo*>(data);
    CXToken *tokens;
    unsigned int numTokens;
    ClangRange range(clang_getCursorExtent(cursor));

    CXFile file;
    clang_getFileLocation(clang_getCursorLocation(cursor),&file,nullptr,nullptr,nullptr);
    if (!file) {
        kDebug() << "Couldn't find file associated with default parameter cursor!";
        //We keep going, because getting an error because we accidentally duplicated
        //a default parameter is better than deleting a default parameter
    }
    QString fileName = ClangString(clang_getFileName(file)).toString();

    //Clang doesn't make a distinction between the default arguments being in
    //the declaration or definition, and the default arguments don't have lexical
    //parents. So this range check is the only thing that really works.
    if ((info->fileName.isEmpty() || fileName == info->fileName) && info->range.contains(range.toSimpleRange())) {
        clang_tokenize(info->unit, range.range(), &tokens, &numTokens);
        for (unsigned int i = 0; i < numTokens; i++) {
            info->stringParts.append(ClangString(clang_getTokenSpelling(info->unit, tokens[i])).toString());
        }

        clang_disposeTokens(info->unit, tokens, numTokens);
    }
    return CXChildVisit_Continue;
}

}

QVector<QString> ClangUtils::getDefaultArguments(CXCursor cursor, DefaultArgumentsMode mode)
{
    if (!CursorKindTraits::isFunction(clang_getCursorKind(cursor))) {
        return QVector<QString>();
    }

    int numArgs = clang_Cursor_getNumArguments(cursor);
    QVector<QString> arguments(mode == FixedSize ? numArgs : 0);
    QString fileName;
    CXFile file;
    clang_getFileLocation(clang_getCursorLocation(cursor),&file,nullptr,nullptr,nullptr);
    if (!file) {
        kDebug() << "Couldn't find file associated with default parameter cursor!";
        //The empty string serves as a wildcard string, because it's better to
        //duplicate a default parameter than delete one
    } else {
        fileName = ClangString(clang_getFileName(file)).toString();
    }

    FunctionInfo info{ClangRange(clang_getCursorExtent(cursor)).toSimpleRange(), fileName,
                      clang_Cursor_getTranslationUnit(cursor), QStringList()};

    for (int i = 0; i < numArgs; i++) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);
        info.stringParts.clear();
        clang_visitChildren(arg, paramVisitor, &info);

        //Clang includes the equal sign sometimes, but not other times.
        if (!info.stringParts.isEmpty() && info.stringParts.first() == QString('=')) {
            info.stringParts.removeFirst();
        }
        //Clang seems to include the , or ) at the end of the param, so delete that
        if (!info.stringParts.isEmpty() && (info.stringParts.last() == QString(",") || info.stringParts.last() == QString(")"))) {
            info.stringParts.removeLast();
        }

        const QString result = info.stringParts.join(QString());
        if (mode == FixedSize) {
            arguments.replace(i, result);
        } else if (!result.isEmpty()) {
            arguments << result;
        }
    }
    return arguments;
}


bool ClangUtils::isConstMethod(CXCursor cursor)
{
    if (clang_getCursorKind(cursor) != CXCursor_CXXMethod) {
        return false;
    }

#if CINDEX_VERSION_MINOR >= 24
    return clang_CXXMethod_isConst(cursor);
#else
    // The clang-c API currently doesn't provide access to a function declaration's
    //const qualifier. This parses the Unified Symbol Resolution to retrieve that information.
    //However, since the USR is undocumented, this might break in the future.
    QString usr = ClangString(clang_getCursorUSR(cursor)).toString();
    if (usr.length() >= 2 && usr.at(usr.length() - 2) == '#') {
        return ((usr.at(usr.length() - 1).toAscii()) - '0') & 0x1;
    } else {
        return false;
    }
#endif
}

constexpr bool isScopeKind(CXCursorKind kind)
{
    return kind == CXCursor_Namespace || kind == CXCursor_StructDecl ||
           kind == CXCursor_UnionDecl || kind == CXCursor_ClassDecl  ||
           kind == CXCursor_ClassTemplate || kind == CXCursor_ClassTemplatePartialSpecialization;
}

QString ClangUtils::getScope(CXCursor cursor)
{
    QStringList scope;
    CXCursor destContext = clang_getCanonicalCursor(clang_getCursorLexicalParent(cursor));
    CXCursor search = clang_getCursorSemanticParent(cursor);
    while (isScopeKind(clang_getCursorKind(search)) && !clang_equalCursors(search, destContext)) {
        scope.prepend(ClangString(clang_getCursorSpelling(search)).toString());
        search = clang_getCursorSemanticParent(search);
    }
    return scope.join("::");
}

QString ClangUtils::getCursorSignature(CXCursor cursor, const QString& scope, QVector<QString> defaultArgs)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    //Get the return type
    QStringList parts;
    if (kind != CXCursor_Constructor && kind != CXCursor_Destructor) {
        parts.append(ClangString(clang_getTypeSpelling(clang_getCursorResultType(cursor))).toString());
        parts.append(" ");
    }

    //Build the function name, with scope and parameters
    if (!scope.isEmpty()) {
        parts.append(scope);
        parts.append("::");
    }

    QString functionName = ClangString(clang_getCursorSpelling(cursor)).toString();
    if (functionName.contains('<')) {
        functionName = functionName.left(functionName.indexOf('<'));
    }
    parts.append(functionName);

    //Add the parameters and such
    parts.append(QString('('));
    int numArgs = clang_Cursor_getNumArguments(cursor);
    for (int i = 0; i < numArgs; i++) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);
        QString id = ClangString(clang_getCursorDisplayName(arg)).toString();
        QString type = ClangString(clang_getTypeSpelling(clang_getCursorType(arg))).toString();

        //Clang formats pointer types as "t *x" and reference types as "t &x", while
        //KDevelop formats them as "t* x" and "t& x". Make that adjustment.
        if (type.length() > 2 && type.at(type.length()-2) == QChar(' ') &&
            (type.at(type.length()-1) == QChar('*') || type.at(type.length()-1) == QChar('&')))
        {
            type = type.left(type.length() - 2) + type.at(type.length()-1);
        }
        QString defaultArg = (i < defaultArgs.count() && !defaultArgs.at(i).isEmpty()) ? " = " + defaultArgs.at(i) : QString();
        parts.append(type + (id.isEmpty() ? QString() : ' ' + id) + defaultArg + (i < numArgs - 1 ? ", " : ""));
    }

    if (clang_Cursor_isVariadic(cursor)) {
        parts.append(numArgs > 0 ? ", ..." : "...");
    }

    parts.append(QString(')'));

    if (isConstMethod(cursor)) {
        parts.append(" const");
    }

    return parts.join(QString());
}
