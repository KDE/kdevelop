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

#include "parseprojectjob.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iruncontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/iproject.h>
#include <klocalizedstring.h>
#include <interfaces/icompletionsettings.h>
#include <QApplication>

using namespace KDevelop;

bool ParseProjectJob::doKill() {
    kDebug() << "stopping project parse job";
    deleteLater();
    return true;
}

ParseProjectJob::~ParseProjectJob() {
    ICore::self()->languageController()->backgroundParser()->revertAllRequests(this);

    if(ICore::self()->runController()->currentJobs().contains(this))
        ICore::self()->runController()->unregisterJob(this);
}

ParseProjectJob::ParseProjectJob(IProject* project, bool forceUpdate)
{
    connect(project, SIGNAL(destroyed(QObject*)), SLOT(deleteNow()));
    m_project = project;
    m_updated = 0;
    m_forceUpdate = forceUpdate;
    m_totalFiles = project->fileSet().size();

    setCapabilities(Killable);

    setObjectName(i18np("Process 1 file in %2","Process %1 files in %2", m_totalFiles, m_project->name()));
}

void ParseProjectJob::deleteNow() {
    delete this;
}

void ParseProjectJob::updateProgress() {

}

void ParseProjectJob::updateReady(const IndexedString& url, ReferencedTopDUContext topContext) {
    Q_UNUSED(url);
    Q_UNUSED(topContext);
    ++m_updated;
    if(m_updated % ((m_totalFiles / 100)+1) == 0)
        updateProgress();

    if(m_updated >= m_totalFiles)
        deleteLater();
}

void ParseProjectJob::start() {
    if (ICore::self()->shuttingDown()) {
        return;
    }

    kDebug() << "starting project parse job";
    QSet< IndexedString > files = m_project->fileSet();

    TopDUContext::Features processingLevel = files.size() < ICore::self()->languageController()->completionSettings()->minFilesForSimplifiedParsing() ?
                                    TopDUContext::VisibleDeclarationsAndContexts : TopDUContext::SimplifiedVisibleDeclarationsAndContexts;

    if (m_forceUpdate) {
        if (processingLevel & TopDUContext::VisibleDeclarationsAndContexts) {
            processingLevel = TopDUContext::AllDeclarationsContextsAndUses;
        }
        processingLevel = (TopDUContext::Features)(TopDUContext::ForceUpdate | processingLevel);
    }
    // prevent UI-lockup by processing events after some files
    // esp. noticeable when dealing with huge projects
    const int processAfter = 1000;
    int processed = 0;
    foreach(const IndexedString& url, files) {
        ICore::self()->languageController()->backgroundParser()->addDocument( url, processingLevel, BackgroundParser::InitialParsePriority, this );
        ++processed;
        if (processed == processAfter) {
            QApplication::processEvents();
            processed = 0;
        }
    }
}

#include "parseprojectjob.moc"
