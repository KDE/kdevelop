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

#include "parsesession.h"
#include "clangparsingenvironment.h"

#include "../util/clangdebug.h"

using namespace KDevelop;

class ClangParsingEnvironmentFileData : public ParsingEnvironmentFileData
{
public:
    ClangParsingEnvironmentFileData()
        : ParsingEnvironmentFileData()
        , environmentHash(0)
        , tuUrl()
        , quality(ClangParsingEnvironment::Unknown)
    {
    }

    ClangParsingEnvironmentFileData(const ClangParsingEnvironmentFileData& rhs)
        : ParsingEnvironmentFileData(rhs)
        , environmentHash(rhs.environmentHash)
        , tuUrl(rhs.tuUrl)
        , quality(rhs.quality)
    {
    }

    ~ClangParsingEnvironmentFileData() = default;

    uint environmentHash;
    IndexedString tuUrl;
    ClangParsingEnvironment::Quality quality;
};

ClangParsingEnvironmentFile::ClangParsingEnvironmentFile(const IndexedString& url,
                                                         const ClangParsingEnvironment& environment)
    : ParsingEnvironmentFile(*(new ClangParsingEnvironmentFileData), url)
{
  d_func_dynamic()->setClassId(this);
  setEnvironment(environment);
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
        if (env->quality() > d_func()->quality) {
            clangDebug() << "Found better quality environment, require update:" << url()
                << "new environment quality:" << env->quality()
                << "old environment quality:" << d_func()->quality;
            return true;
        }
        if (env->translationUnitUrl() == d_func()->tuUrl && env->hash() != d_func()->environmentHash) {
            clangDebug() << "TU environment changed, require update" << url();
            return true;
        }
    }

    bool ret = KDevelop::ParsingEnvironmentFile::needsUpdate(environment);
    if (ret) {
        clangDebug() << "modification revision requires update:" << url();
    }
    return ret;
}

void ClangParsingEnvironmentFile::setEnvironment(const ClangParsingEnvironment& environment)
{
    d_func_dynamic()->tuUrl = environment.translationUnitUrl();
    d_func_dynamic()->environmentHash = environment.hash();
    d_func_dynamic()->quality = environment.quality();
}

bool ClangParsingEnvironmentFile::matchEnvironment(const ParsingEnvironment* environment) const
{
    return dynamic_cast<const ClangParsingEnvironment*>(environment);
}

ClangParsingEnvironment::Quality ClangParsingEnvironmentFile::environmentQuality() const
{
    return d_func()->quality;
}

uint ClangParsingEnvironmentFile::environmentHash() const
{
    return d_func()->environmentHash;
}

DUCHAIN_DEFINE_TYPE(ClangParsingEnvironmentFile)
