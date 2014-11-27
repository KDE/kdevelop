/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2014  Milian Wolff <mail@milianw.de>
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

#ifndef CLANGPARSINGENVIRONMENTFILE_H
#define CLANGPARSINGENVIRONMENTFILE_H

#include <language/duchain/duchainregister.h>
#include <language/duchain/parsingenvironment.h>

#include <duchain/clangduchainexport.h>

class ClangParsingEnvironment;
class ClangParsingEnvironmentFileData;

class KDEVCLANGDUCHAIN_EXPORT ClangParsingEnvironmentFile : public KDevelop::ParsingEnvironmentFile
{
public:
    using Ptr = QExplicitlySharedDataPointer<ClangParsingEnvironmentFile>;

    ClangParsingEnvironmentFile(const KDevelop::IndexedString& url, const ClangParsingEnvironment& environment,
                                bool isSystemHeader);
    ClangParsingEnvironmentFile(ClangParsingEnvironmentFileData& data);
    ~ClangParsingEnvironmentFile();

    virtual bool needsUpdate(const KDevelop::ParsingEnvironment* environment = 0) const;
    virtual int type() const;

    virtual bool matchEnvironment(const KDevelop::ParsingEnvironment* environment) const override;

    void setEnvironment(const ClangParsingEnvironment& environment);
    void setIsSystemHeader(bool isSystemHeader);

    uint environmentHash() const;
    bool inProject() const;
    bool isSystemHeader() const;

    enum {
        Identity = 142
    };

private:
    DUCHAIN_DECLARE_DATA(ClangParsingEnvironmentFile)
};

DUCHAIN_DECLARE_TYPE(ClangParsingEnvironmentFile)

#endif // CLANGPARSINGENVIRONMENTFILE_H
