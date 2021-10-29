/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLANGPARSEJOB_H
#define CLANGPARSEJOB_H

#include <QHash>

#include <language/backgroundparser/parsejob.h>
#include "duchain/clangparsingenvironment.h"
#include "duchain/unsavedfile.h"
#include "duchain/parsesession.h"

class ClangSupport;

class ClangParseJob : public KDevelop::ParseJob
{
    Q_OBJECT
public:
    ClangParseJob(const KDevelop::IndexedString& url,
                  KDevelop::ILanguageSupport* languageSupport);

    ClangSupport* clang() const;

    enum CustomFeatures {
        Rescheduled = (KDevelop::TopDUContext::LastFeature << 1),
        AttachASTWithoutUpdating = (Rescheduled << 1), ///< Used when context is up to date, but has no AST attached.
        UpdateHighlighting = (AttachASTWithoutUpdating << 1) ///< Used when we only need to update highlighting
    };

protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

    const KDevelop::ParsingEnvironment* environment() const override;

private:
    QExplicitlySharedDataPointer<ParseSessionData> createSessionData() const;

    ClangParsingEnvironment m_environment;
    QVector<UnsavedFile> m_unsavedFiles;
    ParseSessionData::Options m_options;
    bool m_tuDocumentIsUnsaved = false;
    QHash<KDevelop::IndexedString, KDevelop::ModificationRevision> m_unsavedRevisions;
};

#endif // CLANGPARSEJOB_H
