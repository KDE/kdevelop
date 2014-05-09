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

#include <kjob.h>
#include <kurl.h>
#include <language/duchain/indexedstring.h>
#include "../languageexport.h"

namespace KDevelop {
class ReferencedTopDUContext;
class IProject;

///A job that parses all project-files in the given project
///Deletes itself as soon as the project is deleted
class KDEVPLATFORMLANGUAGE_EXPORT ParseProjectJob : public KJob
{
    Q_OBJECT
public:
    ParseProjectJob(KDevelop::IProject* project, bool forceUpdate = false );
    virtual ~ParseProjectJob();
    virtual void start();
    virtual bool doKill();

private Q_SLOTS:
    void deleteNow();
    void updateReady(const KDevelop::IndexedString& url, KDevelop::ReferencedTopDUContext topContext);

private:
    int m_updated;
    int m_totalFiles;
    bool m_forceUpdate;
    KDevelop::IProject* m_project;
    void updateProgress();
};

}

#endif // KDEVPLATFORM_PARSEPROJECTJOB_H
