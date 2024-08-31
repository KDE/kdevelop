/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include <KTextEditor/Document>

#include <QTextStream>
#include <QRegularExpression>

#include <memory>
#include <functional>

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
    const auto documents = ICore::self()->documentController()->openDocuments();
    for (auto* document : documents) {
        auto textDocument = document->textDocument();
        // TODO: Introduce a cache so we don't have to re-read all the open documents
        // which were not changed since the last run
        if (!textDocument || !textDocument->url().isLocalFile() || !textDocument->isModified()) {
            continue;
        }
        if (!DocumentFinderHelpers::mimeTypesList().contains(textDocument->mimeType())) {
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

    auto *info = static_cast<FunctionInfo*>(data);
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
        info->stringParts.append(ClangUtils::getRawContents(info->unit, range.range()));
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

        const auto hasDefault = !info.stringParts.isEmpty();

        //Clang includes the equal sign sometimes, but not other times.
        if (!info.stringParts.isEmpty() && info.stringParts.first() == QLatin1String("=")) {
            info.stringParts.removeFirst();
        }
        //Clang seems to include the , or ) at the end of the param, so delete that
        if (!info.stringParts.isEmpty() &&
            ((info.stringParts.last() == QLatin1String(",")) ||
             (info.stringParts.last() == QLatin1String(")") &&
              // assuming otherwise matching "(" & ")" tokens
              info.stringParts.count(QStringLiteral("(")) != info.stringParts.count(QStringLiteral(")"))))) {
            info.stringParts.removeLast();
        }

        const QString result = info.stringParts.join(QString());
        if (mode == FixedSize) {
            arguments.replace(i, result);
        } else if (!result.isEmpty()) {
            arguments << result;
        } else if (hasDefault) {
            // no string obtained, probably due to a parse error...
            // we have to include some argument, otherwise it's even more confusing to our users
            // furthermore, we cannot even do getRawContents on the arg's cursor, as it's cursor
            // extent stops at the first error...
            arguments << i18n("<parse error>");
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
    return scope.join(QLatin1String("::"));
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

    const auto functionName = ClangString(clang_getCursorSpelling(cursor)).toString();
    if (functionName.contains(QLatin1Char('<')) && !functionName.startsWith(QStringLiteral("operator<"))) {
        stream << QStringView{functionName}.first(functionName.indexOf(QLatin1Char('<')));
    } else {
        stream << functionName;
    }

    //Add the parameters and such
    stream << '(';
    int numArgs ;
    QVector<CXCursor> args;

    // SEE https://bugs.kde.org/show_bug.cgi?id=368544
    // clang_Cursor_getNumArguments returns -1 for FunctionTemplate
    // clang checks if cursor's Decl is ObjCMethodDecl or FunctionDecl
    // CXCursor_FunctionTemplate is neither of them instead it has a FunctionTemplateDecl
    // HACK Get function template arguments by visiting children
    if (kind == CXCursor_FunctionTemplate) {
        clang_visitChildren(cursor, [] (CXCursor cursor, CXCursor /*parent*/, CXClientData data) {
            if (clang_getCursorKind(cursor) == CXCursor_ParmDecl) {
                (static_cast<QVector<CXCursor>*>(data))->push_back(cursor);
            }
            return CXChildVisit_Continue;
        }, &args);
        numArgs = args.size();
    } else {
        numArgs = clang_Cursor_getNumArguments(cursor);
        args.reserve(numArgs);
        for (int i = 0; i < numArgs; i++) {
            CXCursor arg = clang_Cursor_getArgument(cursor, i);
            args.push_back(arg);
        }
    }

    for (int i = 0; i < numArgs; i++) {
        CXCursor arg = args[i];

        //Clang formats pointer types as "t *x" and reference types as "t &x", while
        //KDevelop formats them as "t* x" and "t& x". Make that adjustment.
        const QString type = ClangString(clang_getTypeSpelling(clang_getCursorType(arg))).toString();
        if (type.endsWith(QLatin1String(" *")) || type.endsWith(QLatin1String(" &"))) {
            stream << QStringView{type}.first(type.length() - 2) << type.at(type.length() - 1);
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

    switch (clang_getCursorExceptionSpecificationType(cursor)) {
    case CXCursor_ExceptionSpecificationKind_DynamicNone:
        stream << " throw()";
        break;
    case CXCursor_ExceptionSpecificationKind_BasicNoexcept:
        stream << " noexcept";
        break;
    default:
        break;
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

QString ClangUtils::getRawContents(CXTranslationUnit unit, CXSourceRange range)
{
    const auto rangeStart = clang_getRangeStart(range);
    const auto rangeEnd = clang_getRangeEnd(range);
    CXFile rangeFile;
    unsigned int start, end;
    clang_getFileLocation(rangeStart, &rangeFile, nullptr, nullptr, &start);
    clang_getFileLocation(rangeEnd, nullptr, nullptr, nullptr, &end);

    std::size_t fileSize;
    const char* fileBuffer = clang_getFileContents(unit, rangeFile, &fileSize);
    if (fileBuffer && start < fileSize && end <= fileSize && start < end) {
        return QString::fromUtf8(fileBuffer + start, end - start);
    }
    return QString();
}

bool ClangUtils::isExplicitlyDefaultedOrDeleted(CXCursor cursor)
{
    if (clang_getCursorAvailability(cursor) == CXAvailability_NotAvailable) {
        return true;
    }

    if (clang_CXXMethod_isDefaulted(cursor)) {
        return true;
    }
    return false;
}

void ClangUtils::visitChildren(CXCursor parent, std::function<CXChildVisitResult(CXCursor, CXCursor)> visitor)
{
    static constexpr CXCursorVisitor cVisitor = [](CXCursor cursor, CXCursor parent, CXClientData client_data)
    {
        return (*static_cast<std::function<CXChildVisitResult(CXCursor, CXCursor)>*>(client_data))(cursor, parent);
    };
    clang_visitChildren(parent, cVisitor, &visitor);
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

unsigned int ClangUtils::skipTopCommentBlock(CXTranslationUnit unit, CXFile file)
{
    const auto fileRange = clang_getRange(clang_getLocation(unit, file, 1, 1),
                                          clang_getLocation(unit, file, std::numeric_limits<unsigned>::max(), 1));
    const ClangTokens tokens (unit, fileRange);
    const auto nonCommentToken = std::find_if(tokens.begin(), tokens.end(),
                                    [&](CXToken token) { return clang_getTokenKind(token) != CXToken_Comment; });

    // explicitly handle this case, otherwise we skip the preceding whitespace
    if (nonCommentToken == tokens.begin()) {
        return 1;
    }

    const auto location = (nonCommentToken != tokens.end()) ? clang_getTokenExtent(unit, *nonCommentToken) : fileRange;
    return KTextEditor::Cursor(ClangRange(location).end()).line() + 1;
}
