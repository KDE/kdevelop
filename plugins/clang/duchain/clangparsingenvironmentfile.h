/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGPARSINGENVIRONMENTFILE_H
#define CLANGPARSINGENVIRONMENTFILE_H

#include "clangparsingenvironment.h"

#include <language/duchain/duchainregister.h>
#include "clangprivateexport.h"

class ClangParsingEnvironmentFileData;

class KDEVCLANGPRIVATE_EXPORT ClangParsingEnvironmentFile : public KDevelop::ParsingEnvironmentFile
{
public:
    using Ptr = QExplicitlySharedDataPointer<ClangParsingEnvironmentFile>;

    ClangParsingEnvironmentFile(const KDevelop::IndexedString& url, const ClangParsingEnvironment& environment);
    explicit ClangParsingEnvironmentFile(ClangParsingEnvironmentFileData& data);
    ~ClangParsingEnvironmentFile() override;

    bool needsUpdate(const KDevelop::ParsingEnvironment* environment = nullptr) const override;
    int type() const override;

    bool matchEnvironment(const KDevelop::ParsingEnvironment* environment) const override;

    void setEnvironment(const ClangParsingEnvironment& environment);

    ClangParsingEnvironment::Quality environmentQuality() const;

    size_t environmentHash() const;

    enum {
        Identity = 142
    };

private:
    DUCHAIN_DECLARE_DATA(ClangParsingEnvironmentFile)
};

DUCHAIN_DECLARE_TYPE(ClangParsingEnvironmentFile)

#endif // CLANGPARSINGENVIRONMENTFILE_H
