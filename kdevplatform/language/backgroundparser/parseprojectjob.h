/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
