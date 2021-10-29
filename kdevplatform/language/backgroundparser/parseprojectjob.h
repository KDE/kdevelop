/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PARSEPROJECTJOB_H
#define KDEVPLATFORM_PARSEPROJECTJOB_H

#include <serialization/indexedstring.h>
#include <language/languageexport.h>

#include <KJob>

namespace KDevelop {
class ReferencedTopDUContext;
class IProject;
class ParseProjectJobPrivate;

/// A job that parses currently open files that belong to the given project, or all
/// files that belong to the project if the parseAllProjectSources argument is true.
class KDEVPLATFORMLANGUAGE_EXPORT ParseProjectJob
    : public KJob
{
    Q_OBJECT

public:
    explicit ParseProjectJob(KDevelop::IProject* project, bool forceUpdate = false,
                             bool parseAllProjectSources = false);
    ~ParseProjectJob() override;
    void start() override;
    bool doKill() override;

private Q_SLOTS:
    void queueFilesToParse();
    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& topContext);

private:
    const QScopedPointer<class ParseProjectJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ParseProjectJob)
};
}

#endif // KDEVPLATFORM_PARSEPROJECTJOB_H
