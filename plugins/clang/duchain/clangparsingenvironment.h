/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLANGPARSINGENVIRONMENT_H
#define CLANGPARSINGENVIRONMENT_H

#include <util/path.h>
#include <language/duchain/parsingenvironment.h>

#include "clangprivateexport.h"

#include "clangsettings/clangsettingsmanager.h"

class KDEVCLANGPRIVATE_EXPORT ClangParsingEnvironment : public KDevelop::ParsingEnvironment
{
public:
    ~ClangParsingEnvironment() override = default;
    int type() const override;

    /**
     * Sets the list of project paths.
     *
     * Any include path outside these project paths is considered
     * to be a system include.
     */
    void setProjectPaths(const KDevelop::Path::List& projectPaths);
    KDevelop::Path::List projectPaths() const;

    /**
     * Add the given list of @p include paths to this environment.
     */
    void addIncludes(const KDevelop::Path::List& includes);

    /**
     * Add the given list of @p framework-directories to this environment.
     */
    void addFrameworkDirectories(const KDevelop::Path::List& frameworkDirectories);

    struct IncludePaths
    {
        /// This list contains all include paths outside the known projects paths.
        KDevelop::Path::List system;
        /// This list contains all include paths inside the known projects paths.
        KDevelop::Path::List project;
    };
    /**
     * Returns the list of includes, split into a list of system includes and project includes.
     */
    IncludePaths includes() const;

    struct FrameworkDirectories
    {
        /// This list contains all framework directories outside the known projects paths.
        KDevelop::Path::List system;
        /// This list contains all framework directories inside the known projects paths.
        KDevelop::Path::List project;
    };
    /**
     * Returns the list of framework directories, split into a list of system paths and project paths.
     */
    FrameworkDirectories frameworkDirectories() const;

    void addDefines(const QHash<QString, QString>& defines);
    QMap<QString, QString> defines() const;

    void setPchInclude(const KDevelop::Path& path);
    KDevelop::Path pchInclude() const;

    void setWorkingDirectory(const KDevelop::Path& path);
    KDevelop::Path workingDirectory() const;

    void setTranslationUnitUrl(const KDevelop::IndexedString& url);
    KDevelop::IndexedString translationUnitUrl() const;

    enum Quality
    {
        Unknown,
        Source,
        BuildSystem
    };

    void setQuality(Quality quality);
    Quality quality() const;

    void setParserSettings(const ParserSettings& arguments);

    ParserSettings parserSettings() const;
    void addParserArguments(const QString &parserArguments);

    /**
     * Hash all contents of this environment and return the result.
     *
     * This is useful for a quick comparison, and enough to store on-disk
     * to figure out if the environment changed or not.
     */
    size_t hash() const;

    bool operator==(const ClangParsingEnvironment& other) const;
    bool operator!=(const ClangParsingEnvironment& other) const
    {
        return !(*this == other);
    }

private:
    KDevelop::Path::List m_projectPaths;
    KDevelop::Path::List m_includes;
    KDevelop::Path::List m_frameworkDirectories;
    // NOTE: As elements in QHash stored in an unordered sequence, we're using QMap instead
    QMap<QString, QString> m_defines;
    KDevelop::Path m_pchInclude;
    KDevelop::Path m_workingDirectory;
    KDevelop::IndexedString m_tuUrl;
    Quality m_quality = Unknown;
    ParserSettings m_parserSettings;
};

#endif // CLANGPARSINGENVIRONMENT_H
