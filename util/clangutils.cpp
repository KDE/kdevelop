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

    //Clang doesn't make a distinction between the default arguments being in
    //the declaration or definition, and the default arguments don't have lexical
    //parents. So this is the only thing that really works.
    if (info->range.contains(range.toSimpleRange())) {
        clang_tokenize(info->unit, range.range(), &tokens, &numTokens);
        for (unsigned int i = 0; i < numTokens; i++) {
            info->stringParts.append(ClangString(clang_getTokenSpelling(info->unit, tokens[i])).toString());
        }

        clang_disposeTokens(info->unit, tokens, numTokens);
    }
    return CXChildVisit_Continue;
}

}

QVector<QString> ClangUtils::getDefaultArguments(CXCursor cursor)
{
    if (!CursorKindTraits::isFunction(clang_getCursorKind(cursor))) {
        return QVector<QString>();
    }

    int numArgs = clang_Cursor_getNumArguments(cursor);
    QVector<QString> res(numArgs);
    FunctionInfo info{ClangRange(clang_getCursorExtent(cursor)).toSimpleRange(), clang_Cursor_getTranslationUnit(cursor), QStringList()};

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

        res.replace(i, info.stringParts.join(QString()));
    }

    return res;
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


QString ClangUtils::getCursorSignature(CXCursor cursor, CXCursor destContext, QVector<QString> defaultArgs)
{
    QStringList scope;
    CXCursor search = clang_getCursorSemanticParent(cursor);
    while (isScopeKind(clang_getCursorKind(search)) && !clang_equalCursors(search, destContext)) {
        scope.prepend(ClangString(clang_getCursorSpelling(search)).toString() + QString("::"));
        search = clang_getCursorSemanticParent(search);
    }

    QString prefix = scope.join(QString());
    return ClangUtils::getCursorSignature(cursor, prefix, defaultArgs);
}


QString ClangUtils::getCursorSignature(CXCursor cursor, QString& prefix, QVector<QString> defaultArgs)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    //Get the return type
    QStringList parts;
    if (kind != CXCursor_Constructor && kind != CXCursor_Destructor) {
        parts.append(ClangString(clang_getTypeSpelling(clang_getCursorResultType(cursor))).toString());
        parts.append(" ");
    }

    //Build the function name, with scope and parameters
    parts.append(prefix);
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
        QString defaultArg = (i < defaultArgs.count() && !defaultArgs.at(i).isEmpty()) ? " = " + defaultArgs.at(i) : QString();
        parts.append(type + ' ' + id + defaultArg + (i < numArgs - 1 ? ", " : ""));
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

Path::List ClangUtils::defaultIncludeDirectories()
{
    static Path::List includePaths;

    if (!includePaths.isEmpty()) {
        return includePaths;
    }

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    // The following command will spit out a bnuch of information we don't care
    // about before spitting out the include paths.  The parts we care about
    // look like this:
    // #include "..." search starts here:
    // #include <...> search starts here:
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/i486-linux-gnu
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/backward
    //  /usr/local/include
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/include
    //  /usr/include
    // End of search list.
    proc.start("clang++", {"-std=c++11", "-xc++", "-E", "-v", "/dev/null"});
    if (!proc.waitForStarted(1000) || !proc.waitForFinished(1000)) {
        return {};
    }

    // We'll use the following constants to know what we're currently parsing.
    enum Status {
        Initial,
        FirstSearch,
        Includes,
        Finished
    };
    Status mode = Initial;

    foreach(const QString& line, QString::fromLocal8Bit(proc.readAllStandardOutput()).split('\n')) {
        switch (mode) {
            case Initial:
                if (line.indexOf("#include \"...\"") != -1) {
                    mode = FirstSearch;
                }
                break;
            case FirstSearch:
                if (line.indexOf("#include <...>") != -1) {
                    mode = Includes;
                    break;
                }
            case Includes:
                //if (!line.indexOf(QDir::separator()) == -1 && line != "." ) {
                //Detect the include-paths by the first space that is prepended. Reason: The list may contain relative paths like "."
                if (!line.startsWith(' ') ) {
                    // We've reached the end of the list.
                    mode = Finished;
                } else {
                    // This is an include path, add it to the list.
                    includePaths << Path(QDir::cleanPath(line.trimmed()));
                }
                break;
            default:
                break;
        }
        if (mode == Finished) {
            break;
        }
    }

    return includePaths;
}
