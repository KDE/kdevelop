/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    size_t environmentHash;
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
            clangDebug() << "TU environment changed, require update" << url() << "TU url:" << env->translationUnitUrl() << "old hash:" << d_func()->environmentHash << "new hash:" << env->hash();
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

size_t ClangParsingEnvironmentFile::environmentHash() const
{
    return d_func()->environmentHash;
}

DUCHAIN_DEFINE_TYPE(ClangParsingEnvironmentFile)
