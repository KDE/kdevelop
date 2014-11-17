/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CLANGPARSINGENVIRONMENT_H
#define CLANGPARSINGENVIRONMENT_H

#include <util/path.h>
#include <language/duchain/parsingenvironment.h>

#include <duchain/clangduchainexport.h>

class KDEVCLANGDUCHAIN_EXPORT ClangParsingEnvironment : public KDevelop::ParsingEnvironment
{
public:
    virtual ~ClangParsingEnvironment() = default;
    virtual int type() const override;

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

    void addDefines(const QHash<QString, QString>& defines);
    QHash<QString, QString> defines() const;

    void setPchInclude(const KDevelop::Path& path);
    KDevelop::Path pchInclude() const;

    void setProjectKnown(bool known);
    bool projectKnown() const;

    /**
     * Hash all contents of this environment and return the result.
     *
     * This is useful for a quick comparison, and enough to store on-disk
     * to figure out if the environment changed or not.
     */
    uint hash() const;

    bool operator==(const ClangParsingEnvironment& other) const;
    bool operator!=(const ClangParsingEnvironment& other) const
    {
        return !(*this == other);
    }

private:
    KDevelop::Path::List m_projectPaths;
    KDevelop::Path::List m_includes;
    QHash<QString, QString> m_defines;
    KDevelop::Path m_pchInclude;
    bool m_projectKnown = false;
};

#endif // CLANGPARSINGENVIRONMENT_H
