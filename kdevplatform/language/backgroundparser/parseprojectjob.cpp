/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "parseprojectjob.h"

#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/icompletionsettings.h>

#include <language/backgroundparser/backgroundparser.h>

#include <KLocalizedString>

#include <QCoreApplication>
#include <QPointer>
#include <QSet>
#include <QTimer>

using namespace KDevelop;

class KDevelop::ParseProjectJobPrivate
{
public:
    explicit ParseProjectJobPrivate(bool forceUpdate, bool parseAllProjectSources)
        : forceUpdate(forceUpdate)
        , parseAllProjectSources{parseAllProjectSources}
    {
    }

    const bool forceUpdate;
    const bool parseAllProjectSources;
    int fileCountLeftToParse = 0;
    QSet<IndexedString> filesToParse;
};

bool ParseProjectJob::doKill()
{
    qCDebug(LANGUAGE) << "stopping project parse job";
    ICore::self()->languageController()->backgroundParser()->revertAllRequests(this);
    return true;
}

ParseProjectJob::~ParseProjectJob() = default;

ParseProjectJob::ParseProjectJob(IProject* project, bool forceUpdate, bool parseAllProjectSources)
    : d_ptr{new ParseProjectJobPrivate(forceUpdate, parseAllProjectSources)}
{
    Q_D(ParseProjectJob);

    if (parseAllProjectSources) {
        d->filesToParse = project->fileSet();
    } else {
        // In case we don't want to parse the whole project, still add all currently open files that belong to the project to the background-parser
        const auto documents = ICore::self()->documentController()->openDocuments();
        const auto projectFiles = project->fileSet();
        for (auto* document : documents) {
            const auto path = IndexedString(document->url());
            if (projectFiles.contains(path)) {
                d->filesToParse.insert(path);
            }
        }
    }
    d->fileCountLeftToParse = d->filesToParse.size();

    setCapabilities(Killable);

    setObjectName(i18np("Process 1 file in %2", "Process %1 files in %2", d->filesToParse.size(), project->name()));
}

void ParseProjectJob::updateReady(const IndexedString& url, const ReferencedTopDUContext& topContext)
{
    Q_D(ParseProjectJob);

    Q_UNUSED(url);
    Q_UNUSED(topContext);
    --d->fileCountLeftToParse;
    Q_ASSERT(d->fileCountLeftToParse >= 0);
    if (d->fileCountLeftToParse == 0) {
        deleteLater();
    }
}

void ParseProjectJob::start()
{
    Q_D(ParseProjectJob);

    if (d->filesToParse.isEmpty()) {
        deleteLater();
        return;
    }

    qCDebug(LANGUAGE) << "starting project parse job";
    // Avoid calling QCoreApplication::processEvents() directly in start() to prevent
    // a crash in RunController::checkState().
    QTimer::singleShot(0, this, &ParseProjectJob::queueFilesToParse);
}

void ParseProjectJob::queueFilesToParse()
{
    Q_D(ParseProjectJob);

    const auto isJobKilled = [this] {
        if (Q_UNLIKELY(isFinished())) {
            qCDebug(LANGUAGE) << "Aborting queuing project files to parse."
                                 " This job has been killed:" << objectName();
            return true;
        }
        return false;
    };

    if (isJobKilled()) {
        return;
    }

    TopDUContext::Features processingLevel = d->filesToParse.size() <
                                             ICore::self()->languageController()->completionSettings()->
                                             minFilesForSimplifiedParsing() ?
                                             TopDUContext::VisibleDeclarationsAndContexts : TopDUContext::
                                             SimplifiedVisibleDeclarationsAndContexts;
    TopDUContext::Features openDocumentProcessingLevel{TopDUContext::AllDeclarationsContextsAndUses};

    if (d->forceUpdate) {
        if (processingLevel & TopDUContext::VisibleDeclarationsAndContexts) {
            processingLevel = TopDUContext::AllDeclarationsContextsAndUses;
        }
        processingLevel |= TopDUContext::ForceUpdate;
        openDocumentProcessingLevel |= TopDUContext::ForceUpdate;
    }

    if (auto currentDocument = ICore::self()->documentController()->activeDocument()) {
        const auto path = IndexedString(currentDocument->url());
        const auto fileIt = d->filesToParse.constFind(path);
        if (fileIt != d->filesToParse.cend()) {
            ICore::self()->languageController()->backgroundParser()->addDocument(path,
                    openDocumentProcessingLevel, BackgroundParser::BestPriority, this);
            d->filesToParse.erase(fileIt);
        }
    }

    int priority{BackgroundParser::InitialParsePriority};
    const int openDocumentPriority{10};
    if (d->parseAllProjectSources) {
        // Add all currently open files that belong to the project to the
        // background-parser, so that they'll be parsed first of all.
        const auto documents = ICore::self()->documentController()->openDocuments();
        for (auto* document : documents) {
            const auto path = IndexedString(document->url());
            const auto fileIt = d->filesToParse.constFind(path);
            if (fileIt != d->filesToParse.cend()) {
                ICore::self()->languageController()->backgroundParser()->addDocument(path,
                        openDocumentProcessingLevel, openDocumentPriority, this);
                d->filesToParse.erase(fileIt);
            }
        }
    } else {
        // In this case the constructor inserts only open documents into d->filesToParse.
        processingLevel = openDocumentProcessingLevel;
        priority = openDocumentPriority;
    }

    // prevent UI-lockup by processing events after some files
    // esp. noticeable when dealing with huge projects
    const int processAfter = 1000;
    int processed = 0;
    // guard against reentrancy issues, see also bug 345480
    auto crashGuard = QPointer<ParseProjectJob> {this};
    for (const IndexedString& url : std::as_const(d->filesToParse)) {
        ICore::self()->languageController()->backgroundParser()->addDocument(url, processingLevel,
                                                                             priority,
                                                                             this);
        ++processed;
        if (processed == processAfter) {
            QCoreApplication::processEvents();
            if (Q_UNLIKELY(!crashGuard)) {
                qCDebug(LANGUAGE) << "Aborting queuing project files to parse."
                                     " This job has been destroyed.";
                return;
            }
            if (isJobKilled()) {
                return;
            }
            processed = 0;
        }
    }

    d->filesToParse = {}; // free memory or prevent detaching
}

#include "moc_parseprojectjob.cpp"
