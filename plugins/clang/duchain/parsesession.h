/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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

#include "clangparsingenvironment.h"
#include "unsavedfile.h"

class ClangIndex;

class KDEVCLANGPRIVATE_EXPORT ParseSessionData : public KDevelop::IAstContainer
{
public:
    using Ptr = QExplicitlySharedDataPointer<ParseSessionData>;

    enum Option {
        NoOption,                     ///< No special options
        SkipFunctionBodies,           ///< Pass CXTranslationUnit_SkipFunctionBodies (likely unwanted)
        PrecompiledHeader             ///< Pass CXTranslationUnit_PrecompiledPreamble and others to cache precompiled headers
    };
    Q_DECLARE_FLAGS(Options, Option)

    /**
     * Parse the given @p contents.
     *
     * @param unsavedFiles Optional unsaved document contents from the editor.
     */
    ParseSessionData(const QVector<UnsavedFile>& unsavedFiles, ClangIndex* index,
                     const ClangParsingEnvironment& environment, Options options = Options());

    ~ParseSessionData();

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
    QVector<KDevelop::ProblemPointer> m_diagnosticsCache;
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
    Q_DISABLE_COPY(ParseSession);

    ParseSessionData::Ptr d;

};

#endif // PARSESESSION_H
