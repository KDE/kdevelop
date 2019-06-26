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

#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/icompletionsettings.h>

#include <language/backgroundparser/backgroundparser.h>

#include <KLocalizedString>

#include <QApplication>
#include <QPointer>
#include <QSet>

using namespace KDevelop;

class KDevelop::ParseProjectJobPrivate
{
public:
    ParseProjectJobPrivate(IProject* project, bool forceUpdate, bool forceAll)
        : forceUpdate(forceUpdate)
        , forceAll(forceAll)
        , project(project)
    {
    }

    int updated = 0;
    bool forceUpdate;
    bool forceAll;
    KDevelop::IProject* project;
    QSet<IndexedString> filesToParse;
};

bool ParseProjectJob::doKill()
{
    qCDebug(LANGUAGE) << "stopping project parse job";
    deleteLater();
    return true;
}

ParseProjectJob::~ParseProjectJob()
{
    ICore::self()->languageController()->backgroundParser()->revertAllRequests(this);

    if (ICore::self()->runController()->currentJobs().contains(this))
        ICore::self()->runController()->unregisterJob(this);
}

ParseProjectJob::ParseProjectJob(IProject* project, bool forceUpdate, bool forceAll)
    : d_ptr(new ParseProjectJobPrivate(project, forceUpdate, forceAll))
{
    Q_D(ParseProjectJob);

    connect(project, &IProject::destroyed, this, &ParseProjectJob::deleteNow);

    if (forceAll || ICore::self()->projectController()->parseAllProjectSources()) {
        d->filesToParse = project->fileSet();
    } else {
        // In case we don't want to parse the whole project, still add all currently open files that belong to the project to the background-parser
        const auto documents = ICore::self()->documentController()->openDocuments();
        for (auto* document : documents) {
            const auto path = IndexedString(document->url());
            if (project->fileSet().contains(path)) {
                d->filesToParse.insert(path);
            }
        }
    }

    setCapabilities(Killable);

    setObjectName(i18np("Process 1 file in %2", "Process %1 files in %2", d->filesToParse.size(), d->project->name()));
}

void ParseProjectJob::deleteNow()
{
    delete this;
}

void ParseProjectJob::updateProgress()
{
}

void ParseProjectJob::updateReady(const IndexedString& url, const ReferencedTopDUContext& topContext)
{
    Q_D(ParseProjectJob);

    Q_UNUSED(url);
    Q_UNUSED(topContext);
    ++d->updated;
    if (d->updated % ((d->filesToParse.size() / 100) + 1) == 0)
        updateProgress();

    if (d->updated >= d->filesToParse.size())
        deleteLater();
}

void ParseProjectJob::start()
{
    Q_D(ParseProjectJob);

    if (ICore::self()->shuttingDown()) {
        return;
    }

    if (d->filesToParse.isEmpty()) {
        deleteLater();
        return;
    }

    qCDebug(LANGUAGE) << "starting project parse job";

    TopDUContext::Features processingLevel = d->filesToParse.size() <
                                             ICore::self()->languageController()->completionSettings()->
                                             minFilesForSimplifiedParsing() ?
                                             TopDUContext::VisibleDeclarationsAndContexts : TopDUContext::
                                             SimplifiedVisibleDeclarationsAndContexts;

    if (d->forceUpdate) {
        if (processingLevel & TopDUContext::VisibleDeclarationsAndContexts) {
            processingLevel = TopDUContext::AllDeclarationsContextsAndUses;
        }
        processingLevel = ( TopDUContext::Features )(TopDUContext::ForceUpdate | processingLevel);
    }

    if (auto currentDocument = ICore::self()->documentController()->activeDocument()) {
        const auto path = IndexedString(currentDocument->url());
        auto fileIt = d->filesToParse.find(path);
        if (fileIt != d->filesToParse.end()) {
            ICore::self()->languageController()->backgroundParser()->addDocument(path,
                                                                                 TopDUContext::AllDeclarationsContextsAndUses, BackgroundParser::BestPriority,
                                                                                 this);
            d->filesToParse.erase(fileIt);
        }
    }

    // Add all currently open files that belong to the project to the background-parser, so that they'll be parsed first of all
    const auto documents = ICore::self()->documentController()->openDocuments();
    for (auto* document : documents) {
        const auto path = IndexedString(document->url());
        auto fileIt = d->filesToParse.find(path);
        if (fileIt != d->filesToParse.end()) {
            ICore::self()->languageController()->backgroundParser()->addDocument(path,
                                                                                 TopDUContext::AllDeclarationsContextsAndUses, 10,
                                                                                 this);
            d->filesToParse.erase(fileIt);
        }
    }

    if (!d->forceAll && !ICore::self()->projectController()->parseAllProjectSources()) {
        return;
    }

    // prevent UI-lockup by processing events after some files
    // esp. noticeable when dealing with huge projects
    const int processAfter = 1000;
    int processed = 0;
    // guard against reentrancy issues, see also bug 345480
    auto crashGuard = QPointer<ParseProjectJob> {this};
    for (const IndexedString& url : qAsConst(d->filesToParse)) {
        ICore::self()->languageController()->backgroundParser()->addDocument(url, processingLevel,
                                                                             BackgroundParser::InitialParsePriority,
                                                                             this);
        ++processed;
        if (processed == processAfter) {
            QApplication::processEvents();
            if (!crashGuard) {
                return;
            }
            processed = 0;
        }
    }
}
