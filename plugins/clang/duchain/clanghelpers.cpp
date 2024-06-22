/*
    SPDX-FileCopyrightText: 2014 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clanghelpers.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/backgroundparser/urlparselock.h>

#include "builder.h"
#include "parsesession.h"
#include "clangparsingenvironmentfile.h"
#include "clangindex.h"
#include "clangducontext.h"
#include "util/clangdebug.h"
#include "util/clangtypes.h"

#include "libclang_include_path.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <algorithm>

#if HAVE_DLFCN
#include <dlfcn.h>
#endif

using namespace KDevelop;

namespace {

CXChildVisitResult visitCursor(CXCursor cursor, CXCursor, CXClientData data)
{
    if (cursor.kind != CXCursor_InclusionDirective) {
        return CXChildVisit_Continue;
    }

    auto imports = static_cast<Imports*>(data);
    CXFile file = clang_getIncludedFile(cursor);
    if(!file){
        return CXChildVisit_Continue;
    }

    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile parentFile;
    uint line, column;
    clang_getFileLocation(location, &parentFile, &line, &column, nullptr);

    const auto parentFileImports = imports->values(parentFile);
    for (const auto& import : parentFileImports) {
        // clang_getInclusions doesn't include the same import twice, so we shouldn't do it too.
        if (import.file == file) {
            return CXChildVisit_Continue;
        }
    }

    imports->insert(parentFile, {file, CursorInRevision(line-1, column-1)});

    return CXChildVisit_Recurse;
}

ReferencedTopDUContext createTopContext(const IndexedString& path, const ClangParsingEnvironment& environment)
{
    auto* file = new ClangParsingEnvironmentFile(path, environment);
    ReferencedTopDUContext context = new ClangTopDUContext(path, RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
    DUChain::self()->addDocumentChain(context);
    context->updateImportsCache();
    return context;
}

}

Imports ClangHelpers::tuImports(CXTranslationUnit tu)
{
    Imports imports;

    // Intentionally don't use clang_getInclusions here, as it skips already visited inclusions
    // which makes TestDUChain::testNestedImports fail
    CXCursor tuCursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(tuCursor, &visitCursor, &imports);

    return imports;
}

bool importLocationLessThan(const Import& lhs, const Import& rhs)
{
    return lhs.location.line < rhs.location.line;
}

ReferencedTopDUContext ClangHelpers::buildDUChain(CXFile file, const Imports& imports, const ParseSession& session,
                                                  TopDUContext::Features features, IncludeFileContexts& includedFiles,
                                                  const UnsavedRevisions& unsavedRevisions,
                                                  const KDevelop::IndexedString& parseDocument, ClangIndex* index,
                                                  const std::function<bool()>& abortFunction)
{
    if (includedFiles.contains(file)) {
        return {};
    }

    if (abortFunction && abortFunction()) {
        return {};
    }

    // prevent recursion
    includedFiles.insert(file, {});

    // ensure DUChain for imports are built properly, and in correct order
    QList<Import> sortedImports = imports.values(file);
    std::sort(sortedImports.begin(), sortedImports.end(), importLocationLessThan);

    for (const auto& import : std::as_const(sortedImports)) {
        buildDUChain(import.file, imports, session, features, includedFiles, unsavedRevisions, parseDocument, index,
                     abortFunction);
    }

    const QFileInfo pathInfo(ClangString(clang_getFileName(file)).toString());
    const IndexedString path(pathInfo.canonicalFilePath());
    if (path.isEmpty()) {
        // may happen when the file gets removed before the job is run
        return {};
    }

    const auto& environment = session.environment();

    bool update = false;
    UrlParseLock urlLock(path);
    ReferencedTopDUContext context;
    {
        DUChainWriteLocker lock;
        context = DUChain::self()->chainForDocument(path, &environment);
        if (!context) {
            context = ::createTopContext(path, environment);
        } else {
            update = true;
        }

        includedFiles.insert(file, context);

        auto envFile = dynamic_cast<ClangParsingEnvironmentFile*>(context->parsingEnvironmentFile().data());
        if (!envFile) {
            qCWarning(KDEV_CLANG) << "no suitable environment file for context" << file
                                  << context->parsingEnvironmentFile();
            return context;
        }

        if (update) {
            /*
             * The features of the parse request are meant for the parseDocument.
             * We don't want to apply ForceUpdate to all imports,
             * except when ForceUpdateRecursive is set!
             */
            const auto pathFeatures = [features, path, parseDocument]() {
                if (path == parseDocument || features.testFlag(TopDUContext::ForceUpdateRecursive)) {
                    return features;
                }
                auto ret = features;
                return ret.setFlag(TopDUContext::ForceUpdate, false);
            };
            if (!envFile->needsUpdate(&environment) && envFile->featuresSatisfied(pathFeatures())) {
                return context;
            }

            // TODO: don't attempt to update if this environment is worse quality than the outdated one
            if (index && envFile->environmentQuality() < environment.quality()) {
                index->pinTranslationUnitForUrl(environment.translationUnitUrl(), path);
            }
            envFile->setEnvironment(environment);

            envFile->clearModificationRevisions();
            context->clearImportedParentContexts();
        }
        context->setFeatures(features);

        for (const auto& import : std::as_const(sortedImports)) {
            auto ctx = includedFiles.value(import.file);
            if (!ctx) {
                // happens for cyclic imports
                continue;
            }
            context->addImportedParentContext(ctx, import.location);
            envFile->addModificationRevisions(ctx->parsingEnvironmentFile()->allModificationRevisions());
        }
        context->updateImportsCache();

        // prefer the editor modification revision, instead of the on-disk revision
        auto it = unsavedRevisions.find(path);
        if (it == unsavedRevisions.end()) {
            envFile->setModificationRevision(ModificationRevision::revisionForFile(path));
        } else {
            envFile->setModificationRevision(*it);
        }
    }

    const auto problems = session.problemsForFile(file);
    {
        DUChainWriteLocker lock;
        context->setProblems(problems);
    }

    Builder::visit(session.unit(), file, includedFiles, update);

    DUChain::self()->emitUpdateReady(path, context);

    return context;
}

DeclarationPointer ClangHelpers::findDeclaration(CXSourceLocation location, const QualifiedIdentifier& id, const ReferencedTopDUContext& top)
{
    if (!top) {
        // may happen for cyclic includes
        return {};
    }

    auto cursor = CursorInRevision(ClangLocation(location));
    DUChainReadLocker lock;

    if (!id.isEmpty()) {
        const auto& decls = top->findDeclarations(id);
        for (Declaration* decl : decls) {
            if (decl->range().contains(cursor) ||
                (decl->range().isEmpty() && decl->range().start == cursor))
            {
                return DeclarationPointer(decl);
            }
        }
    }

    // there was no match based on the IDs, try the classical
    // range based search (very slow)

    Q_ASSERT(top);
    if (DUContext *local = top->findContextAt(cursor)) {
        if (local->owner() && local->owner()->range().contains(cursor)) {
           return DeclarationPointer(local->owner());
        }
        return DeclarationPointer(local->findDeclarationAt(cursor));
    }
    return {};
}

DeclarationPointer ClangHelpers::findDeclaration(CXCursor cursor, const IncludeFileContexts& includes)
{
    auto location = clang_getCursorLocation(cursor);
    CXFile file = nullptr;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    if (!file) {
        return {};
    }

    // build a qualified identifier by following the chain of semantic parents
    QList<Identifier> ids;
    CXCursor currentCursor = cursor;
    while (currentCursor.kind != CXCursor_TranslationUnit &&
           currentCursor.kind != CXCursor_InvalidFile)
    {
        ids << Identifier(ClangString(clang_getCursorSpelling(currentCursor)).toString());
        currentCursor = clang_getCursorSemanticParent(currentCursor);
    }
    QualifiedIdentifier qid;
    for (int i = ids.size()-1; i >= 0; --i)
    {
        qid.push(ids[i]);
    }

    return findDeclaration(location, qid, includes.value(file));
}

DeclarationPointer ClangHelpers::findDeclaration(CXType type, const IncludeFileContexts& includes)
{
    CXCursor cursor = clang_getTypeDeclaration(type);
    return findDeclaration(cursor, includes);
}

DeclarationPointer ClangHelpers::findForwardDeclaration(CXType type, DUContext* context, CXCursor cursor)
{
    if(type.kind != CXType_Record && type.kind != CXType_ObjCInterface && type.kind != CXType_ObjCClass){
        return {};
    }

    auto qualifiedIdentifier = QualifiedIdentifier(ClangString(clang_getTypeSpelling(type)).toString());

    DUChainReadLocker lock;
    const auto decls = context->findDeclarations(qualifiedIdentifier,
        CursorInRevision(ClangLocation(clang_getCursorLocation(cursor)))
    );

    for (auto decl : decls) {
        if (decl->isForwardDeclaration()) {
            return DeclarationPointer(decl);
        }
    }
    return {};
}

RangeInRevision ClangHelpers::cursorSpellingNameRange(CXCursor cursor, const Identifier& id)
{
    auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRangeInRevision();
#if CINDEX_VERSION_MINOR < 29
    auto kind = clang_getCursorKind(cursor);
    // Clang used to report invalid ranges for destructors and methods like 'operator='
    if (kind == CXCursor_Destructor || kind == CXCursor_CXXMethod) {
        range.end.column = range.start.column + id.toString().length();
    }
#endif
    Q_UNUSED(id);
    return range;
}

QStringList ClangHelpers::headerExtensions()
{
    static const QStringList headerExtensions = {
            QStringLiteral("h"),
            QStringLiteral("H"),
            QStringLiteral("hh"),
            QStringLiteral("hxx"),
            QStringLiteral("hpp"),
            QStringLiteral("tlh"),
            QStringLiteral("cuh"),
            QStringLiteral("h++"),
    };
    return headerExtensions;
}

QStringList ClangHelpers::sourceExtensions()
{
    static const QStringList sourceExtensions = {
        QStringLiteral("c"),
        QStringLiteral("cc"),
        QStringLiteral("cpp"),
        QStringLiteral("c++"),
        QStringLiteral("cxx"),
        QStringLiteral("C"),
        QStringLiteral("cu"),
        QStringLiteral("m"),
        QStringLiteral("mm"),
        QStringLiteral("M"),
        QStringLiteral("inl"),
        QStringLiteral("_impl.h"),
    };
    return sourceExtensions;
}

bool ClangHelpers::isSource(const QString& path)
{
    const auto& extensions = sourceExtensions();
    return std::any_of(extensions.constBegin(), extensions.constEnd(),
                       [&](const QString& ext) { return path.endsWith(ext); });
}

bool ClangHelpers::isHeader(const QString& path)
{
    const auto& extensions = headerExtensions();
    return std::any_of(extensions.constBegin(), extensions.constEnd(),
                       [&](const QString& ext) { return path.endsWith(ext); });
}

QString ClangHelpers::clangVersion()
{
    static const auto clangVersion = []() -> QString {
        // NOTE: The apidocs for clang_getClangVersion() clearly state it shouldn't be used for parsing
        // but there's no other way to retrieve the Clang version at runtime at this point...
        const ClangString version(clang_getClangVersion());
        clangDebug() << "Full Clang version:" << version;

        // samples:
        //   clang version 6.0.1 (trunk 321709) (git@github.com:llvm-mirror/llvm.git 5136df4d089a086b70d452160ad5451861269498)
        //   clang version 7.0.0-svn341916-1~exp1~20180911115939.26 (branches/release_70)
        //   Ubuntu clang version 11.0.0-2
        QRegularExpression re(QStringLiteral("clang version (\\d+\\.\\d+\\.\\d+)"));
        const auto match = re.match(version.toString());
        if (!match.hasMatch())
            return {};

        return match.captured(1); // return e.g. 7.0.0
    }();
    return clangVersion;
}

static QString majorClangVersion()
{
    const QString version = ClangHelpers::clangVersion();
    return version.left(version.indexOf(QLatin1Char{'.'}));
}

bool ClangHelpers::isValidClangBuiltingIncludePath(const QString& path)
{
    return QFile::exists(path + QLatin1String("/cpuid.h"));
}

QString ClangHelpers::clangBuiltinIncludePath()
{
    // use a lambda to store the result in a static variable which can be
    // returned without recomputing the string on subsequent calls.
    static const auto dir = []() -> QString {
        auto dir = QString::fromUtf8(qgetenv("KDEV_CLANG_BUILTIN_DIR"));
        if (!dir.isEmpty() && isValidClangBuiltingIncludePath(dir)) {
            clangDebug() << "Using dir from $KDEV_CLANG_BUILTIN_DIR:" << dir;
            return dir;
        }

        // Since https://github.com/llvm/llvm-project/commit/e1b88c8a09be25b86b13f98755a9bd744b4dbf14
        // Clang's resource directory includes only the major version.
        const QString majorVersion = majorClangVersion();
        const QString versionSubdir = majorVersion.toInt() >= 16 ? majorVersion : clangVersion();

#ifdef Q_OS_WIN32
        // attempt to use the bundled copy on Windows
        dir = QDir::cleanPath(
            QStringLiteral("%1/../lib/clang/%2/include").arg(QCoreApplication::applicationDirPath(), versionSubdir));
        if (isValidClangBuiltingIncludePath(dir)) {
            clangDebug() << "Using builtin dir:" << dir;
            return dir;
        }
#elif defined(Q_OS_UNIX)
        // a clang version upgrade since we were last built can
        // cause problems if the "clang/$fullversion/include" path component
        // changed. Try to generate the correct builtin_dir for the current
        // major.minor.patchlevel version: pop the last 2 components then
        // chdir through with the updated version directory.
        dir = QDir::cleanPath(QStringLiteral(KDEV_CLANG_BUILTIN_DIR "/../../%1/include").arg(versionSubdir));
        if (isValidClangBuiltingIncludePath(dir)) {
            clangDebug() << "Using builtin dir:" << dir;
            return dir;
        }
#endif

#if HAVE_DLFCN
        // maybe the location of clang changed, try to use the library path instead
        // we find it by pass any symbol in libclang to dladdr
        Dl_info info;
        if (dladdr(reinterpret_cast<void*>(&clang_getClangVersion), &info)) {
            // "../lib/" because libclang may be in lib64/ while includes are in lib/
            for (auto pathTemplate :
                 {QLatin1String{"%1/../clang/%2/include"}, QLatin1String{"%1/../../lib/clang/%2/include"}}) {
                dir = QDir::cleanPath(pathTemplate.arg(QString::fromUtf8(info.dli_fname), versionSubdir));
                if (isValidClangBuiltingIncludePath(dir)) {
                    clangDebug() << "Using builtin dir:" << dir;
                    return dir;
                }
            }
        }
#endif

        clangDebug() << "Using builtin dir:" << KDEV_CLANG_BUILTIN_DIR;
        return QString::fromUtf8(KDEV_CLANG_BUILTIN_DIR);
    }();
    return dir;
}
