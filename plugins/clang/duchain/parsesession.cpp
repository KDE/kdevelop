/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2013 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "parsesession.h"
#include <QStandardPaths>
#include "clangdiagnosticevaluator.h"
#include "todoextractor.h"
#include "clanghelpers.h"
#include "clangindex.h"
#include "clangparsingenvironment.h"
#include "util/clangdebug.h"
#include "util/clangtypes.h"
#include "util/clangutils.h"
#include "headerguardassistant.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/codegen/coderepresentation.h>

#include <KShell>

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

#include <algorithm>

using namespace KDevelop;

namespace {

QVector<QByteArray> extraArgs()
{
    const auto extraArgsString = QString::fromLatin1(qgetenv("KDEV_CLANG_EXTRA_ARGUMENTS"));
    const auto extraArgs = KShell::splitArgs(extraArgsString);

    // transform to list of QByteArrays
    QVector<QByteArray> result;
    result.reserve(extraArgs.size());
    for (const QString& arg : extraArgs) {
        result << arg.toLatin1();
    }
    clangDebug() << "Passing extra arguments to clang:" << result;

    return result;
}

void removeHarmfulArguments(QVector<QByteArray>& arguments)
{
    const auto shouldRemove = [](const QByteArray& argument) {
        constexpr const char* toRemove[] = {
            "-Werror",
            "-Wlogical-op", // prevent runtime -Wunknown-warning-option warning because Clang does not support this flag
#if CINDEX_VERSION_MINOR < 100 // FIXME https://bugs.llvm.org/show_bug.cgi?id=35333
            "-Wdocumentation",
#endif
        };
        return std::any_of(std::cbegin(toRemove), std::cend(toRemove), [&argument](const char* rm) {
            return argument == rm;
        });
    };

    const auto end = arguments.end();
    const auto logicalEnd = std::remove_if(arguments.begin(), end, shouldRemove);
    arguments.erase(logicalEnd, end);
}

void sanitizeArguments(QVector<QByteArray>& arguments)
{
    removeHarmfulArguments(arguments);

    // We remove the -Werror flag, and replace -Werror=foo by -Wfoo.
    // Warning as error may cause problem to the clang parser.
    const auto asError = QByteArrayLiteral("-Werror=");
    // Silence common warning that arises when we parse as a GCC-lookalike.
    // Note how clang warns us about emulating GCC, which is exactly what we want here.
    const auto noGnuZeroVaridicMacroArguments = QByteArrayLiteral("-Wno-gnu-zero-variadic-macro-arguments");
    bool noGnuZeroVaridicMacroArgumentsFound = false;
    bool isCpp = false;
    const auto sizedDealloc = QByteArrayLiteral("-fsized-deallocation");
    bool sizedDeallocFound = false;
    for (auto& argument : arguments) {
        if (argument.startsWith(asError)) {
            // replace -Werror=foo by -Wfoo
            argument.remove(2, asError.length() - 2);
        } else if (!noGnuZeroVaridicMacroArgumentsFound && argument == noGnuZeroVaridicMacroArguments) {
            noGnuZeroVaridicMacroArgumentsFound = true;
        } else if (!isCpp && argument == "-xc++") {
            isCpp = true;
        } else if (!sizedDeallocFound && argument == sizedDealloc) {
            sizedDeallocFound = true;
        }
    }

    if (!noGnuZeroVaridicMacroArgumentsFound) {
        arguments.append(noGnuZeroVaridicMacroArguments);
    }
    if (isCpp && !sizedDeallocFound) {
        // see e.g.
        // https://youtrack.jetbrains.com/issue/CPP-29091/In-template-call-to-builtinoperatordelete-selects-non-usual-deallocation-function-gcc-12#focus=Comments-27-6067190.0-0
        arguments.append(sizedDealloc);
    }
}

QVector<QByteArray> argsForSession(const QString& path, ParseSessionData::Options options, const ParserSettings& parserSettings)
{
    QMimeDatabase db;
    if (db.mimeTypeForFile(path).name() == QLatin1String("text/x-objcsrc")) {
        return {QByteArrayLiteral("-xobjective-c++")};
    }

    // TODO: No proper mime type detection possible yet
    // cf. https://bugs.freedesktop.org/show_bug.cgi?id=26913
    if (path.endsWith(QLatin1String(".cl"), Qt::CaseInsensitive)) {
        return {QByteArrayLiteral("-xcl")};
    }

    // TODO: No proper mime type detection possible yet
    // cf. https://bugs.freedesktop.org/show_bug.cgi?id=23700
    if (path.endsWith(QLatin1String(".cu"), Qt::CaseInsensitive) ||
        path.endsWith(QLatin1String(".cuh"), Qt::CaseInsensitive)) {
        auto result = parserSettings.toClangAPI();
        result.append(QByteArrayLiteral("-xcuda"));
        return result;
    }

    if (parserSettings.parserOptions.isEmpty()) {
        // The parserOptions can be empty for some unit tests that use ParseSession directly
        auto defaultArguments = ClangSettingsManager::self()->parserSettings(path).toClangAPI();

        defaultArguments.append(QByteArrayLiteral("-nostdinc"));
        defaultArguments.append(QByteArrayLiteral("-nostdinc++"));
        defaultArguments.append(QByteArrayLiteral("-xc++"));

        sanitizeArguments(defaultArguments);
        return defaultArguments;
    }

    auto result = parserSettings.toClangAPI();
    result.append(QByteArrayLiteral("-nostdinc"));
    if (parserSettings.isCpp()) {
        result.append(QByteArrayLiteral("-nostdinc++"));
    }

    if (options & ParseSessionData::PrecompiledHeader) {
        result.append(parserSettings.isCpp() ? QByteArrayLiteral("-xc++-header") : QByteArrayLiteral("-xc-header"));

        sanitizeArguments(result);
        return result;
    }

    result.append(parserSettings.isCpp() ? QByteArrayLiteral("-xc++") : QByteArrayLiteral("-xc"));

    sanitizeArguments(result);
    return result;
}

void addIncludes(QVector<const char*>* args, QVector<QByteArray>* otherArgs,
                 const Path::List& includes, const char* cliSwitch)
{
    for (const Path& url : includes) {
        if (url.isEmpty()) {
            continue;
        }

        QFileInfo info(url.toLocalFile());
        QByteArray path = url.toLocalFile().toUtf8();

        if (info.isFile()) {
            path.prepend("-include");
        } else {
            path.prepend(cliSwitch);
        }
        otherArgs->append(path);
        args->append(path.constData());
    }
}

void addFrameworkDirectories(QVector<const char*>* args, QVector<QByteArray>* otherArgs,
                 const Path::List& frameworkDirectories, const char* cliSwitch)
{
    for (const Path& url : frameworkDirectories) {
        if (url.isEmpty()) {
            continue;
        }

        QFileInfo info(url.toLocalFile());
        if (!info.isDir()) {
            qCWarning(KDEV_CLANG) << "supposed framework directory is not a directory:" << url.pathOrUrl();
            continue;
        }
        QByteArray path = url.toLocalFile().toUtf8();

        otherArgs->append(path);
        args->append(cliSwitch);
        args->append(path.constData());
    }
}

QVector<CXUnsavedFile> toClangApi(const QVector<UnsavedFile>& unsavedFiles)
{
    QVector<CXUnsavedFile> unsaved;
    unsaved.reserve(unsavedFiles.size());
    std::transform(unsavedFiles.begin(), unsavedFiles.end(),
                std::back_inserter(unsaved),
                [] (const UnsavedFile& file) { return file.toClangApi(); });
    return unsaved;
}

bool notClangAndGccVersionAtLeast(const QMap<QString, QString>& defines, int minGccMajorVersion)
{
    return !defines.contains(QStringLiteral("__clang__"))
        && defines.value(QStringLiteral("__GNUC__")).toInt() >= minGccMajorVersion;
}

bool hasQtIncludes(const Path::List& includePaths)
{
    return std::find_if(includePaths.begin(), includePaths.end(), [] (const Path& path) {
        return path.lastPathSegment() == QLatin1String("QtCore");
    }) != includePaths.end();
}

}

ParseSessionData::ParseSessionData(const QVector<UnsavedFile>& unsavedFiles, ClangIndex* index,
                                   const ClangParsingEnvironment& environment, Options options)
    : m_file(nullptr)
    , m_unit(nullptr)
{
    unsigned int flags = CXTranslationUnit_DetailedPreprocessingRecord
#if CINDEX_VERSION_MINOR >= 34
        | CXTranslationUnit_KeepGoing
#endif
    ;
    if (options.testFlag(SkipFunctionBodies)) {
        flags |= CXTranslationUnit_SkipFunctionBodies;
    }
    if (options.testFlag(PrecompiledHeader)) {
        flags |= CXTranslationUnit_ForSerialization;
    } else if (environment.quality() == ClangParsingEnvironment::Unknown) {
        flags |= CXTranslationUnit_Incomplete;
    }
    if (options.testFlag(OpenedInEditor)) {
        flags |= CXTranslationUnit_CacheCompletionResults
#if CINDEX_VERSION_MINOR >= 32
              |  CXTranslationUnit_CreatePreambleOnFirstParse
#endif
              |  CXTranslationUnit_PrecompiledPreamble;
    }

    const auto totalSize = [](const auto& paths) {
        return paths.system.size() + paths.project.size();
    };

    const auto tuUrl = environment.translationUnitUrl();
    Q_ASSERT(!tuUrl.isEmpty());

    const auto arguments = argsForSession(tuUrl.str(), options, environment.parserSettings());
    QVector<const char*> clangArguments;

    const auto& includes = environment.includes();
    const auto& frameworkDirectories = environment.frameworkDirectories();
    const auto& pchInclude = environment.pchInclude();

    static const auto extraArgs = ::extraArgs();

    // uses QByteArray as smart-pointer for const char* ownership
    QVector<QByteArray> smartArgs;

    /// the number of arguments, for which one element is appended to smartArgs and two elements - to clangArguments
    const auto argumentCountToDouble = totalSize(frameworkDirectories) + pchInclude.isValid() + 6;
    smartArgs.reserve(totalSize(includes) + argumentCountToDouble);
    clangArguments.reserve(arguments.size() + smartArgs.capacity() + argumentCountToDouble + extraArgs.size());

    std::transform(arguments.constBegin(), arguments.constEnd(),
                   std::back_inserter(clangArguments),
                   [] (const QByteArray &argument) { return argument.constData(); });

    // NOTE: the PCH include must come before all other includes!
    if (pchInclude.isValid()) {
        clangArguments << "-include";
        QByteArray pchFile = pchInclude.toLocalFile().toUtf8();
        smartArgs << pchFile;
        clangArguments << pchFile.constData();
    }

    if (notClangAndGccVersionAtLeast(environment.defines(), 7)) {
        const auto compatFile =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                   QStringLiteral("kdevclangsupport/gccCompatibility/additional_floating_types.h"))
                .toUtf8();
        if (!compatFile.isEmpty()) {
            smartArgs << compatFile;
            clangArguments << "-include" << compatFile.constData();
        }
    }

    if (hasQtIncludes(includes.system)) {
        auto wrappedQtHeaders =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                   QStringLiteral("kdevclangsupport/wrappedQtHeaders/QtCore/qobjectdefs.h"))
                .toUtf8();
        if (!wrappedQtHeaders.isEmpty()) {
            // add /path/to/wrappedQtHeaders/QtCore and /path/to/wrappedQtHeaders to the SYSTEM include search path
            for (auto counter = 0; counter < 2; ++counter) {
                // remove "/qobjectdefs.h" during the first iteration and "/QtCore" during the second one
                wrappedQtHeaders.truncate(wrappedQtHeaders.lastIndexOf('/'));
                smartArgs << wrappedQtHeaders;
                clangArguments << "-isystem" << wrappedQtHeaders.constData();
            }
        }
    }

    addIncludes(&clangArguments, &smartArgs, includes.system, "-isystem");
    addIncludes(&clangArguments, &smartArgs, includes.project, "-I");
    addFrameworkDirectories(&clangArguments, &smartArgs, frameworkDirectories.system, "-iframework");
    addFrameworkDirectories(&clangArguments, &smartArgs, frameworkDirectories.project, "-F");

    // libclang cannot find it's builtin dir automatically, we have to specify it manually
    smartArgs << ClangHelpers::clangBuiltinIncludePath().toUtf8();
    clangArguments << "-isystem" << smartArgs.last().constData();

    {
        smartArgs << writeDefinesFile(environment.defines());
        clangArguments << "-imacros" << smartArgs.last().constData();
    }

    if (!environment.workingDirectory().isEmpty()) {
        QByteArray workingDirectory = environment.workingDirectory().toLocalFile().toUtf8();
        workingDirectory.prepend("-working-directory");
        smartArgs << workingDirectory;
        clangArguments << workingDirectory.constData();
    }

    // append extra args from environment variable
    for (const QByteArray& arg : extraArgs) {
        clangArguments << arg.constData();
    }

    QVector<CXUnsavedFile> unsaved;
    //For PrecompiledHeader, we don't want unsaved contents (and contents.isEmpty())
    if (!options.testFlag(PrecompiledHeader)) {
        unsaved = toClangApi(unsavedFiles);
    }

    // debugging: print hypothetical clang invocation including args (for easy c&p for local testing)
    if (qEnvironmentVariableIsSet("KDEV_CLANG_DISPLAY_ARGS")) {
        QTextStream out(stdout);
        out << "Invocation: clang";
        for (const auto& arg : std::as_const(clangArguments)) {
            out << " " << arg;
        }
        out << " " << tuUrl.byteArray().constData() << "\n";
    }

    const CXErrorCode code = clang_parseTranslationUnit2(
        index->index(), tuUrl.byteArray().constData(),
        clangArguments.constData(), clangArguments.size(),
        unsaved.data(), unsaved.size(),
        flags,
        &m_unit
    );
    if (code != CXError_Success) {
        qCWarning(KDEV_CLANG) << "clang_parseTranslationUnit2 return with error code" << code;
        if (!qEnvironmentVariableIsSet("KDEV_CLANG_DISPLAY_DIAGS")) {
            qCWarning(KDEV_CLANG) << "  (start KDevelop with `KDEV_CLANG_DISPLAY_DIAGS=1 kdevelop` to see more diagnostics)";
        }
    }

    if (m_unit) {
        setUnit(m_unit);
        m_environment = environment;

        if (options.testFlag(PrecompiledHeader)) {
            clang_saveTranslationUnit(m_unit, QByteArray(tuUrl.byteArray() + ".pch").constData(), CXSaveTranslationUnit_None);
        }
    } else {
        qCWarning(KDEV_CLANG) << "Failed to parse translation unit:" << tuUrl;
    }
}

ParseSessionData::~ParseSessionData()
{
    clang_disposeTranslationUnit(m_unit);
}

QByteArray ParseSessionData::writeDefinesFile(const QMap<QString, QString>& defines)
{
    m_definesFile.open();
    Q_ASSERT(m_definesFile.isWritable());

    {
        QTextStream definesStream(&m_definesFile);
        // don't show warnings about redefined macros
        definesStream << "#pragma clang system_header\n";
        for (auto it = defines.begin(); it != defines.end(); ++it) {
            if (it.key().startsWith(QLatin1String("__has_include("))
                || it.key().startsWith(QLatin1String("__has_include_next(")))
            {
                continue;
            }
            definesStream << QLatin1String("#define ") << it.key() << ' ' << it.value() << '\n';
        }
        if (notClangAndGccVersionAtLeast(defines, 11)) {
            /* fake GCC compatibility for __malloc__ attribute with arguments to silence warnings like this:

             /usr/include/stdlib.h:566:5: error: use of undeclared identifier '__builtin_free'; did you mean
             '__builtin_frexp'? /usr/include/stdlib.h:566:5: note: '__builtin_frexp' declared here
             /usr/include/stdlib.h:566:5: error: '__malloc__' attribute takes no arguments
             /usr/include/stdlib.h:570:14: error: '__malloc__' attribute takes no arguments
             /usr/include/stdlib.h:799:6: error: use of undeclared identifier '__builtin_free'; did you mean
             '__builtin_frexp'? /usr/include/stdlib.h:566:5: note: '__builtin_frexp' declared here
             /usr/include/stdlib.h:799:6: error: '__malloc__' attribute takes no arguments
             */
            definesStream << QLatin1String("#define __malloc__(...) __malloc__\n");
        }
    }
    m_definesFile.close();

    if (qEnvironmentVariableIsSet("KDEV_CLANG_DISPLAY_DEFINES")) {
        QFile f(m_definesFile.fileName());
        f.open(QIODevice::ReadOnly);
        Q_ASSERT(f.isReadable());
        QTextStream out(stdout);
        out << "Defines file: " << f.fileName() << "\n"
            << f.readAll() << f.size()
            << "\n VS defines:" << defines.size() << "\n";
    }

    return m_definesFile.fileName().toUtf8();
}

void ParseSessionData::setUnit(CXTranslationUnit unit)
{
    m_unit = unit;
    m_diagnosticsCache.clear();
    if (m_unit) {
        const ClangString unitFile(clang_getTranslationUnitSpelling(unit));
        m_file = clang_getFile(m_unit, unitFile.c_str());
    } else {
        m_file = nullptr;
    }
}

ClangParsingEnvironment ParseSessionData::environment() const
{
    return m_environment;
}

ParseSession::ParseSession(const ParseSessionData::Ptr& data)
    : d(data)
{
    if (d) {
        ENSURE_CHAIN_NOT_LOCKED
        d->m_mutex.lock();
    }
}

ParseSession::~ParseSession()
{
    if (d) {
        d->m_mutex.unlock();
    }
}

void ParseSession::setData(const ParseSessionData::Ptr& data)
{
    if (data == d) {
        return;
    }

    if (d) {
        d->m_mutex.unlock();
    }

    d = data;

    if (d) {
        ENSURE_CHAIN_NOT_LOCKED
        d->m_mutex.lock();
    }
}

ParseSessionData::Ptr ParseSession::data() const
{
    return d;
}

IndexedString ParseSession::languageString()
{
    static const IndexedString lang("Clang");
    return lang;
}

ClangProblem::Ptr ParseSession::getOrCreateProblem(int indexInTU, CXDiagnostic diagnostic) const
{
    if (!d) {
        return {};
    }

    auto& problem = d->m_diagnosticsCache[indexInTU];
    if (!problem) {
        problem = ClangDiagnosticEvaluator::createProblem(diagnostic, d->m_unit);
    }
    return problem;
}

ClangProblem::Ptr ParseSession::createExternalProblem(int indexInTU,
                                                      CXDiagnostic diagnostic,
                                                      const KLocalizedString& descriptionTemplate,
                                                      int childProblemFinalLocationIndex) const
{
    // Make a copy of the original (cached) problem since it is modified later
    auto problem = ClangProblem::Ptr(new ClangProblem(*getOrCreateProblem(indexInTU, diagnostic)));

    // Insert a copy of the parent problem (without child problems) as the first
    // child problem to preserve its location.
    auto* problemCopy = new ClangProblem();
    problemCopy->setSource(problem->source());
    problemCopy->setFinalLocation(problem->finalLocation());
    problemCopy->setFinalLocationMode(problem->finalLocationMode());
    problemCopy->setDescription(problem->description());
    problemCopy->setExplanation(problem->explanation());
    problemCopy->setSeverity(problem->severity());

    auto childProblems = problem->diagnostics();
    childProblems.prepend(IProblem::Ptr(problemCopy));
    problem->setDiagnostics(childProblems);

    // Override the problem's finalLocation with that of the child problem in this document.
    // This is required to make the problem show up in the problem reporter for this
    // file, since it filters by finalLocation. It will also lead the user to the correct
    // location when clicking the problem and cause proper error highlighting.
    int index = (childProblemFinalLocationIndex >= 0) ?
                (1 + childProblemFinalLocationIndex) :
                (childProblems.size() - 1);
    problem->setFinalLocation(childProblems[index]->finalLocation());

    problem->setDescription(descriptionTemplate.subs(problem->description()).toString());

    return problem;
}

QList<ClangProblem::Ptr> ParseSession::createRequestedHereProblems(int indexInTU, CXDiagnostic diagnostic, CXFile file) const
{
    QList<ClangProblem::Ptr> results;

    auto childDiagnostics = clang_getChildDiagnostics(diagnostic);
    auto numChildDiagnostics = clang_getNumDiagnosticsInSet(childDiagnostics);
    for (uint j = 0; j < numChildDiagnostics; ++j) {
        auto childDiagnostic = clang_getDiagnosticInSet(childDiagnostics, j);
        CXSourceLocation childLocation = clang_getDiagnosticLocation(childDiagnostic);
        CXFile childDiagnosticFile;
        clang_getFileLocation(childLocation, &childDiagnosticFile, nullptr, nullptr, nullptr);
        if (childDiagnosticFile == file) {
            QString description = ClangString(clang_getDiagnosticSpelling(childDiagnostic)).toString();
            if (description.endsWith(QLatin1String("requested here"))) {
                // Note: Using the index j here assumes a 1:1 mapping from clang child diagnostics to KDevelop
                // problem diagnostics (i.e., child problems). If we wanted to avoid making this assumption, we'd have
                // to use ClangDiagnosticEvaluator::createProblem() first and then search within its
                // child problems to find the correct index.
                results << createExternalProblem(indexInTU, diagnostic, ki18n("Requested here: %1"), j);
            }
        }
    }

    return results;
}

QList<ProblemPointer> ParseSession::problemsForFile(CXFile file) const
{
    if (!d) {
        return {};
    }

    QList<ProblemPointer> problems;

    // extra clang diagnostics
    const uint numDiagnostics = clang_getNumDiagnostics(d->m_unit);
    problems.reserve(numDiagnostics);
    d->m_diagnosticsCache.resize(numDiagnostics);

    for (uint i = 0; i < numDiagnostics; ++i) {
        auto diagnostic = clang_getDiagnostic(d->m_unit, i);

        CXSourceLocation location = clang_getDiagnosticLocation(diagnostic);
        CXFile diagnosticFile;
        clang_getFileLocation(location, &diagnosticFile, nullptr, nullptr, nullptr);

        const auto requestedHereProblems = createRequestedHereProblems(i, diagnostic, file);
        for (const auto& ptr : requestedHereProblems) {
            problems.append(static_cast<const ProblemPointer&>(ptr));
        }

        // missing-include problems are so severe in clang that we always propagate
        // them to this document, to ensure that the user will see the error.
        if (diagnosticFile != file && ClangDiagnosticEvaluator::diagnosticType(diagnostic) != ClangDiagnosticEvaluator::IncludeFileNotFoundProblem) {
            continue;
        }

        problems << ((diagnosticFile == file) ?
                     getOrCreateProblem(i, diagnostic) :
                     createExternalProblem(i, diagnostic, ki18n("In included file: %1")));

        clang_disposeDiagnostic(diagnostic);
    }

    // other problem sources

    TodoExtractor extractor(unit(), file);
    problems << extractor.problems();

#if CINDEX_VERSION_MINOR > 30
    // note that the below warning is triggered on every reparse when there is a precompiled preamble
    // see also TestDUChain::testReparseIncludeGuard
    const QString path = QDir(ClangString(clang_getFileName(file)).toString()).canonicalPath();
    const IndexedString indexedPath(path);
    const auto location = clang_getLocationForOffset(d->m_unit, file, 0);
    if (ClangHelpers::isHeader(path) && !clang_isFileMultipleIncludeGuarded(unit(), file)
        && !clang_Location_isInSystemHeader(location)
        // clang_isFileMultipleIncludeGuarded always returns 0 in case our only file is the header
        && !clang_Location_isFromMainFile(location))
    {
        QExplicitlySharedDataPointer<StaticAssistantProblem> problem(new StaticAssistantProblem);
        problem->setSeverity(IProblem::Warning);
        problem->setDescription(i18n("Header is not guarded against multiple inclusions"));
        problem->setExplanation(i18n("The given header is not guarded against multiple inclusions, "
            "either with the conventional #ifndef/#define/#endif macro guards or with #pragma once."));
        const KTextEditor::Range problemRange(0, 0, KDevelop::createCodeRepresentation(indexedPath)->lines(), 0);
        problem->setFinalLocation(DocumentRange{indexedPath, problemRange});
        problem->setSource(IProblem::Preprocessor);
        problem->setSolutionAssistant(KDevelop::IAssistant::Ptr(new HeaderGuardAssistant(d->m_unit, file)));
        problems << problem;
    }
#endif

    return problems;
}

CXTranslationUnit ParseSession::unit() const
{
    return d ? d->m_unit : nullptr;
}

CXFile ParseSession::file(const QByteArray& path) const
{
    return clang_getFile(unit(), path.constData());
}

CXFile ParseSession::mainFile() const
{
    return d ? d->m_file : nullptr;
}

bool ParseSession::reparse(const QVector<UnsavedFile>& unsavedFiles, const ClangParsingEnvironment& environment)
{
    if (!d || environment != d->m_environment) {
        return false;
    }

    auto unsaved = toClangApi(unsavedFiles);

    const auto code = clang_reparseTranslationUnit(d->m_unit, unsaved.size(), unsaved.data(),
                                                   clang_defaultReparseOptions(d->m_unit));
    if (code != CXError_Success) {
        qCWarning(KDEV_CLANG) << "clang_reparseTranslationUnit return with error code" << code;
        // if error code != 0 => clang_reparseTranslationUnit invalidates the old translation unit => clean up
        clang_disposeTranslationUnit(d->m_unit);
        d->setUnit(nullptr);
        return false;
    }

    // update state
    d->setUnit(d->m_unit);
    return true;
}

ClangParsingEnvironment ParseSession::environment() const
{
    if (!d) {
        return {};
    }
    return d->m_environment;
}
