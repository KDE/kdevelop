/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <QList>
#include <QTemporaryFile>

#include <clang-c/Index.h>

#include <serialization/indexedstring.h>

#include <util/path.h>

#include <language/duchain/problem.h>
#include <language/interfaces/iastcontainer.h>

#include "clangprivateexport.h"
#include "clangproblem.h"
#include "clangparsingenvironment.h"
#include "unsavedfile.h"

class ClangIndex;

class KDEVCLANGPRIVATE_EXPORT ParseSessionData : public KDevelop::IAstContainer
{
public:
    using Ptr = QExplicitlySharedDataPointer<ParseSessionData>;

    enum Option {
        NoOption = 0,                     ///< No special options
        SkipFunctionBodies = 1 << 0,      ///< Pass CXTranslationUnit_SkipFunctionBodies (likely unwanted)
        PrecompiledHeader = 1 << 1,       ///< Pass CXTranslationUnit_PrecompiledPreamble and others to cache precompiled headers
        OpenedInEditor = 1 << 2,          ///< File is currently opened in the editor
    };
    Q_DECLARE_FLAGS(Options, Option)

    /**
     * Parse the given @p contents.
     *
     * @param unsavedFiles Optional unsaved document contents from the editor.
     */
    ParseSessionData(const QVector<UnsavedFile>& unsavedFiles, ClangIndex* index,
                     const ClangParsingEnvironment& environment, Options options = Options());

    ~ParseSessionData() override;

    ClangParsingEnvironment environment() const;

private:
    friend class ParseSession;
    void setUnit(CXTranslationUnit unit);
    QByteArray writeDefinesFile(const QMap<QString, QString>& defines);

    QMutex m_mutex;

    CXFile m_file = nullptr;
    CXTranslationUnit m_unit = nullptr;
    ClangParsingEnvironment m_environment;
    /// TODO: share this file for all TUs that use the same defines (probably most in a project)
    ///       best would be a PCH, if possible
    QTemporaryFile m_definesFile;
    // cached ProblemPointer representation for diagnostics
    QVector<ClangProblem::Ptr> m_diagnosticsCache;
};

/**
 * Thread-safe utility class around a CXTranslationUnit.
 *
 * It will lock the mutex of the currently set ParseSessionData and thereby ensure
 * only one ParseSession can operate on a given CXTranslationUnit stored therein.
 */
class KDEVCLANGPRIVATE_EXPORT ParseSession
{
public:
    /**
     * @return a unique identifier for Clang documents.
     */
    static KDevelop::IndexedString languageString();

    /**
     * Initialize a parse session with the given data and, if that data is valid, lock its mutex.
     */
    explicit ParseSession(const ParseSessionData::Ptr& data);
    /**
     * Unlocks the mutex of the currently set ParseSessionData.
     */
    ~ParseSession();

    /**
     * Unlocks the mutex of the currently set ParseSessionData, and instead acquire the lock in @p data.
     */
    void setData(const ParseSessionData::Ptr& data);
    ParseSessionData::Ptr data() const;

    /**
     * @return find the CXFile for the given path.
     */
    CXFile file(const QByteArray& path) const;

    /**
     * @return the CXFile for the first file in this translation unit.
     */
    CXFile mainFile() const;

    QList<KDevelop::ProblemPointer> problemsForFile(CXFile file) const;

    CXTranslationUnit unit() const;

    bool reparse(const QVector<UnsavedFile>& unsavedFiles, const ClangParsingEnvironment& environment);

    ClangParsingEnvironment environment() const;

private:
    Q_DISABLE_COPY(ParseSession)

    ClangProblem::Ptr getOrCreateProblem(int indexInTU, CXDiagnostic diagnostic) const;
    
    ClangProblem::Ptr createExternalProblem(int indexInTU,
                                            CXDiagnostic diagnostic,
                                            const KLocalizedString& descriptionTemplate,
                                            int childProblemFinalLocationIndex = -1) const;
    
    QList<ClangProblem::Ptr> createRequestedHereProblems(int indexInTU, CXDiagnostic diagnostic, CXFile file) const;

    ParseSessionData::Ptr d;

};

#endif // PARSESESSION_H
