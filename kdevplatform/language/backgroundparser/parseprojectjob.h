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

///A job that parses all project-files in the given project either
///when KDevelop is configured to parse all files at project import
///(see ProjectController:parseAllProjectSources()) or when the
///forceAll argument is true. That forceAll argument allows to
///trigger a full project reparse after the initial import, e.g.
///via the project manager's context menu.
///ParseProjectJob instances delete themselves as soon as the project
///is deleted or when a new job is started.
class KDEVPLATFORMLANGUAGE_EXPORT ParseProjectJob
    : public KJob
{
    Q_OBJECT

public:
    explicit ParseProjectJob(KDevelop::IProject* project, bool forceUpdate = false, bool forceAll = false);
    ~ParseProjectJob() override;
    void start() override;
    bool doKill() override;

private Q_SLOTS:
    void deleteNow();
    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& topContext);

private:
    void updateProgress();

private:
    const QScopedPointer<class ParseProjectJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ParseProjectJob)
};
}

#endif // KDEVPLATFORM_PARSEPROJECTJOB_H
