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

#include "../util/clangdebug.h"
#include "../util/clangtypes.h"
#include "../duchain/cursorkindtraits.h"
#include "../duchain/documentfinderhelpers.h"

#include <language/duchain/stringhelpers.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <clang-c/Index.h>

#include <QTextStream>
#include <QRegularExpression>

using namespace KDevelop;

CXCursor ClangUtils::getCXCursor(int line, int column, const CXTranslationUnit& unit, const CXFile& file)
{
    if (!file) {
        clangDebug() << "getCXCursor couldn't find file: " << clang_getFileName(file);
        return clang_getNullCursor();
    }

    CXSourceLocation location = clang_getLocation(unit, file, line + 1, column + 1);

    if (clang_equalLocations(clang_getNullLocation(), location)) {
        clangDebug() << "getCXCursor given invalid position " << line << ", " << column
                << " for file " << clang_getFileName(file);
        return clang_getNullCursor();
    }

    return clang_getCursor(unit, location);
}

QVector<UnsavedFile> ClangUtils::unsavedFiles()
{
    QVector<UnsavedFile> ret;
    foreach(auto document, ICore::self()->documentController()->openDocuments()) {
        auto textDocument = document->textDocument();
        // TODO: Introduce a cache so we don't have to re-read all the open documents
        // which were not changed since the last run
        if (!textDocument || !textDocument->url().isLocalFile()
            || !DocumentFinderHelpers::mimeTypesList().contains(textDocument->mimeType()))
        {
            continue;
        }
        if (!textDocument->isModified()) {
            continue;
        }
        ret << UnsavedFile(textDocument->url().toLocalFile(),
                           textDocument->textLines(textDocument->documentRange()));
    }
    return ret;
}

KTextEditor::Range ClangUtils::rangeForIncludePathSpec(const QString& line, const KTextEditor::Range& originalRange)
{
    static const QRegularExpression pattern(QStringLiteral("^\\s*(#\\s*include|#\\s*import)"));
    if (!line.contains(pattern)) {
        return KTextEditor::Range::invalid();
    }

    KTextEditor::Range range = originalRange;
    int pos = 0;
    char term_char = 0;
    for (; pos < line.size(); ++pos) {
        if (line[pos] == QLatin1Char('"') || line[pos] == QLatin1Char('<')) {
            term_char = line[pos] == QLatin1Char('"') ? '"' : '>';
            range.setStart({ range.start().line(), ++pos });
            break;
        }
    }

    for (; pos < line.size(); ++pos) {
        if (line[pos] == QLatin1Char('\\')) {
            ++pos;
            continue;
        } else if(line[pos] == QLatin1Char(term_char)) {
            range.setEnd({ range.start().line(), pos });
            break;
        }
    }

    return range;
}

namespace {

struct FunctionInfo {
    KTextEditor::Range range;
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
    ClangRange range(clang_getCursorExtent(cursor));

    CXFile file;
    clang_getFileLocation(clang_getCursorLocation(cursor),&file,nullptr,nullptr,nullptr);
    if (!file) {
        clangDebug() << "Couldn't find file associated with default parameter cursor!";
        //We keep going, because getting an error because we accidentally duplicated
        //a default parameter is better than deleting a default parameter
    }
    QString fileName = ClangString(clang_getFileName(file)).toString();

    //Clang doesn't make a distinction between the default arguments being in
    //the declaration or definition, and the default arguments don't have lexical
    //parents. So this range check is the only thing that really works.
    if ((info->fileName.isEmpty() || fileName == info->fileName) && info->range.contains(range.toRange())) {
        const ClangTokens tokens(info->unit, range.range());
        for (CXToken token : tokens) {
            info->stringParts.append(ClangString(clang_getTokenSpelling(info->unit, token)).toString());
        }
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
        clangDebug() << "Couldn't find file associated with default parameter cursor!";
        //The empty string serves as a wildcard string, because it's better to
        //duplicate a default parameter than delete one
    } else {
        fileName = ClangString(clang_getFileName(file)).toString();
    }

    FunctionInfo info{ClangRange(clang_getCursorExtent(cursor)).toRange(), fileName,
                      clang_Cursor_getTranslationUnit(cursor), QStringList()};

    for (int i = 0; i < numArgs; i++) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);
        info.stringParts.clear();
        clang_visitChildren(arg, paramVisitor, &info);

        //Clang includes the equal sign sometimes, but not other times.
        if (!info.stringParts.isEmpty() && info.stringParts.first() == QLatin1String("=")) {
            info.stringParts.removeFirst();
        }
        //Clang seems to include the , or ) at the end of the param, so delete that
        if (!info.stringParts.isEmpty() && (info.stringParts.last() == QLatin1String(",") || info.stringParts.last() == QLatin1String(")"))) {
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

bool ClangUtils::isScopeKind(CXCursorKind kind)
{
    return kind == CXCursor_Namespace || kind == CXCursor_StructDecl ||
           kind == CXCursor_UnionDecl || kind == CXCursor_ClassDecl  ||
           kind == CXCursor_ClassTemplate || kind == CXCursor_ClassTemplatePartialSpecialization;
}

QString ClangUtils::getScope(CXCursor cursor, CXCursor context)
{
    QStringList scope;
    if (clang_Cursor_isNull(context)) {
        context = clang_getCursorLexicalParent(cursor);
    }
    context = clang_getCanonicalCursor(context);
    CXCursor search = clang_getCursorSemanticParent(cursor);
    while (isScopeKind(clang_getCursorKind(search)) && !clang_equalCursors(search, context)) {
        scope.prepend(ClangString(clang_getCursorDisplayName(search)).toString());
        search = clang_getCursorSemanticParent(search);
    }
    return scope.join(QStringLiteral("::"));
}

QString ClangUtils::getCursorSignature(CXCursor cursor, const QString& scope, const QVector<QString>& defaultArgs)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    //Get the return type
    QString ret;
    ret.reserve(128);
    QTextStream stream(&ret);
    if (kind != CXCursor_Constructor && kind != CXCursor_Destructor) {
        stream << ClangString(clang_getTypeSpelling(clang_getCursorResultType(cursor))).toString()
               << ' ';
    }

    //Build the function name, with scope and parameters
    if (!scope.isEmpty()) {
        stream << scope << "::";
    }

    QString functionName = ClangString(clang_getCursorSpelling(cursor)).toString();
    if (functionName.contains(QLatin1Char('<'))) {
        stream << functionName.left(functionName.indexOf(QLatin1Char('<')));
    } else {
        stream << functionName;
    }

    //Add the parameters and such
    stream << '(';
    int numArgs = clang_Cursor_getNumArguments(cursor);
    for (int i = 0; i < numArgs; i++) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);

        //Clang formats pointer types as "t *x" and reference types as "t &x", while
        //KDevelop formats them as "t* x" and "t& x". Make that adjustment.
        const QString type = ClangString(clang_getTypeSpelling(clang_getCursorType(arg))).toString();
        if (type.endsWith(QLatin1String(" *")) || type.endsWith(QLatin1String(" &"))) {
            stream << type.left(type.length() - 2) << type.at(type.length() - 1);
        } else {
            stream << type;
        }

        const QString id = ClangString(clang_getCursorDisplayName(arg)).toString();
        if (!id.isEmpty()) {
            stream << ' ' << id;
        }

        if (i < defaultArgs.count() && !defaultArgs.at(i).isEmpty()) {
            stream << " = " << defaultArgs.at(i);
        }

        if (i < numArgs - 1) {
            stream << ", ";
        }
    }

    if (clang_Cursor_isVariadic(cursor)) {
        if (numArgs > 0) {
            stream << ", ";
        }
        stream << "...";
    }

    stream << ')';

    if (clang_CXXMethod_isConst(cursor)) {
        stream << " const";
    }

    return ret;
}

QStringList ClangUtils::templateArgumentTypes(CXCursor cursor)
{
    CXType typeList = clang_getCursorType(cursor);
    int templateArgCount = clang_Type_getNumTemplateArguments(typeList);
    QStringList types;
    types.reserve(templateArgCount);
    for (int i = 0; i < templateArgCount; ++i) {
        ClangString clangString(clang_getTypeSpelling(clang_Type_getTemplateArgumentAsType(typeList, i)));
        types.append(clangString.toString());
    }

    return types;
}

QByteArray ClangUtils::getRawContents(CXTranslationUnit unit, CXSourceRange range)
{
    const auto rangeStart = clang_getRangeStart(range);
    const auto rangeEnd = clang_getRangeEnd(range);
    unsigned int start, end;
    clang_getFileLocation(rangeStart, nullptr, nullptr, nullptr, &start);
    clang_getFileLocation(rangeEnd, nullptr, nullptr, nullptr, &end);

    QByteArray result;
    const ClangTokens tokens(unit, range);
    for (CXToken token : tokens) {
        const auto location = ClangLocation(clang_getTokenLocation(unit, token));
        unsigned int offset;
        clang_getFileLocation(location, nullptr, nullptr, nullptr, &offset);
        if (offset < start) // TODO: Sometimes hit, see bug 357585
            return {};

        const int fillCharacters = offset - start - result.size();
        Q_ASSERT(fillCharacters >= 0);
        if (fillCharacters < 0)
            return {};

        result.append(QByteArray(fillCharacters, ' '));
        const auto spelling = clang_getTokenSpelling(unit, token);
        result.append(clang_getCString(spelling));
        clang_disposeString(spelling);
    }
    // Clang always appends the full range of the last token, even if this exceeds the end of the requested range.
    // Fix this.
    result.chop(result.size() - (end - start));

    return result;
}

bool ClangUtils::isExplicitlyDefaultedOrDeleted(CXCursor cursor)
{
    if (clang_getCursorAvailability(cursor) == CXAvailability_NotAvailable) {
        return true;
    }

#if CINDEX_VERSION_MINOR >= 34
    if (clang_CXXMethod_isDefaulted(cursor)) {
        return true;
    }
#else
    auto declCursor = clang_getCanonicalCursor(cursor);
    CXTranslationUnit tu = clang_Cursor_getTranslationUnit(declCursor);
    ClangTokens tokens(tu, clang_getCursorExtent(declCursor));
    bool lastTokenWasDeleteOrDefault = false;
    for (auto it = tokens.rbegin(), end = tokens.rend(); it != end; ++it) {
        CXToken token = *it;
        auto kind = clang_getTokenKind(token);
        switch (kind) {
            case CXToken_Comment:
                break;
            case CXToken_Identifier:
            case CXToken_Literal:
                lastTokenWasDeleteOrDefault = false;
                break;
            case CXToken_Punctuation: {
                ClangString spelling(clang_getTokenSpelling(tu, token));
                const char* spellingCStr = spelling.c_str();
                if (strcmp(spellingCStr, ")") == 0) {
                    // a closing parent means we have reached the end of the function parameter list
                    // therefore this function can't be explicitly deleted/defaulted
                    return false;
                } else if (strcmp(spellingCStr, "=") == 0) {
                    if (lastTokenWasDeleteOrDefault) {
                        return true;
                    }
#if CINDEX_VERSION_MINOR < 31
                    // HACK: on old clang versions, we don't get the default/delete
                    //       so there, assume the function is defaulted or deleted
                    //       when the last token is an equal sign
                    if (it == tokens.rbegin()) {
                        return true;
                    }
#endif
                }
                lastTokenWasDeleteOrDefault = false;
                break;
            }
            case CXToken_Keyword: {
                ClangString spelling(clang_getTokenSpelling(tu, token));
                const char* spellingCStr = spelling.c_str();
                if (strcmp(spellingCStr, "default") == 0
#if CINDEX_VERSION_MINOR < 31
                    || strcmp(spellingCStr, "delete") == 0
#endif
                ) {
                    lastTokenWasDeleteOrDefault = true;
                } else {
                    lastTokenWasDeleteOrDefault = false;
                }
                break;
            }
        }
    }
#endif
    return false;
}

KDevelop::ClassFunctionFlags ClangUtils::specialAttributes(CXCursor cursor)
{
    // check for our injected attributes to detect Qt signals and slots
    // see also the contents of wrappedQtHeaders/QtCore/qobjectdefs.h
    ClassFunctionFlags flags = {};
    if (cursor.kind == CXCursor_CXXMethod) {
        clang_visitChildren(cursor, [] (CXCursor cursor, CXCursor /*parent*/, CXClientData data) -> CXChildVisitResult {
            auto& flags = *static_cast<ClassFunctionFlags*>(data);
            switch (cursor.kind) {
            case CXCursor_AnnotateAttr: {
                ClangString attribute(clang_getCursorDisplayName(cursor));
                if (attribute.c_str() == QByteArrayLiteral("qt_signal")) {
                    flags |= FunctionSignalFlag;
                } else if (attribute.c_str() == QByteArrayLiteral("qt_slot")) {
                    flags |= FunctionSlotFlag;
                }
                break;
            }
            case CXCursor_CXXFinalAttr:
                flags |= FinalFunctionFlag;
                break;
            default:
                break;
            }

            return CXChildVisit_Break;
        }, &flags);
    }
    return flags;
}
