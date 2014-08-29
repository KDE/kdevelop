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

#include "clangparsingenvironmentfile.h"

#include <language/duchain/duchainregister.h>

#include "parsesession.h"
#include "clangparsingenvironment.h"

#include "../debug.h"

using namespace KDevelop;

REGISTER_DUCHAIN_ITEM(ClangParsingEnvironmentFile);

class ClangParsingEnvironmentFileData : public ParsingEnvironmentFileData
{
public:
    ClangParsingEnvironmentFileData()
        : ParsingEnvironmentFileData()
        , environmentHash(0)
        , projectWasKnown(false)
        , isSystemHeader(false)
    {
    }

    ClangParsingEnvironmentFileData(const ClangParsingEnvironmentFileData& rhs)
        : ParsingEnvironmentFileData(rhs)
        , environmentHash(rhs.environmentHash)
        , projectWasKnown(rhs.projectWasKnown)
        , isSystemHeader(rhs.isSystemHeader)
    {
    }

    ~ClangParsingEnvironmentFileData() = default;

    uint environmentHash;
    bool projectWasKnown;
    bool isSystemHeader;
};

ClangParsingEnvironmentFile::ClangParsingEnvironmentFile(const IndexedString& url,
                                                         const ClangParsingEnvironment& environment,
                                                         const bool isSystemHeader)
    : ParsingEnvironmentFile(*(new ClangParsingEnvironmentFileData), url)
{
  d_func_dynamic()->setClassId(this);
  setEnvironment(environment);
  setIsSystemHeader(isSystemHeader);
  setLanguage(ParseSession::languageString());
}

ClangParsingEnvironmentFile::ClangParsingEnvironmentFile(ClangParsingEnvironmentFileData& data)
    : ParsingEnvironmentFile(data)
{
}

ClangParsingEnvironmentFile::~ClangParsingEnvironmentFile() = default;

int ClangParsingEnvironmentFile::type() const
{
    return CppParsingEnvironment;
}

bool ClangParsingEnvironmentFile::needsUpdate(const ParsingEnvironment* environment) const
{
    if (environment) {
        Q_ASSERT(dynamic_cast<const ClangParsingEnvironment*>(environment));
        auto env = static_cast<const ClangParsingEnvironment*>(environment);
        if (!d_func()->isSystemHeader && env->hash() != d_func()->environmentHash
            && (env->projectKnown() || env->projectKnown() == d_func()->projectWasKnown))
        {
            debug() << "environment differs, require update:" << url()
                    << "new hash:" << env->hash() << "new project known:" << env->projectKnown()
                    << "old hash:" << d_func()->environmentHash << "old project known:" << d_func()->projectWasKnown;
            return true;
        }
    }

    bool ret = KDevelop::ParsingEnvironmentFile::needsUpdate(environment);
    if (ret) {
        debug() << "modification revision requires update:" << url();
    }
    return ret;
}

void ClangParsingEnvironmentFile::setEnvironment(const ClangParsingEnvironment& environment)
{
    d_func_dynamic()->environmentHash = environment.hash();
    d_func_dynamic()->projectWasKnown = environment.projectKnown();
}

bool ClangParsingEnvironmentFile::matchEnvironment(const ParsingEnvironment* environment) const
{
    return dynamic_cast<const ClangParsingEnvironment*>(environment);
}

uint ClangParsingEnvironmentFile::environmentHash() const
{
    return d_func()->environmentHash;
}

bool ClangParsingEnvironmentFile::inProject() const
{
    return d_func()->projectWasKnown;
}

void ClangParsingEnvironmentFile::setIsSystemHeader(bool isSystemHeader)
{
    d_func_dynamic()->isSystemHeader = isSystemHeader;
}

bool ClangParsingEnvironmentFile::isSystemHeader() const
{
    return d_func()->isSystemHeader;
}
