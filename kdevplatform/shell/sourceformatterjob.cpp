/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sourceformatterjob.h"

#include "sourceformattercontroller.h"

#include <debug.h>

#include <QTextStream>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <sublime/message.h>

using namespace KDevelop;


SourceFormatterJob::SourceFormatterJob(SourceFormatterController* sourceFormatterController)
    : KJob(sourceFormatterController)
    , m_sourceFormatterController(sourceFormatterController)
    , m_workState(WorkIdle)
    , m_fileIndex(0)
{
    setCapabilities(Killable);
    // set name for job listing
    setObjectName(i18n("Reformatting"));

    KDevelop::ICore::self()->uiController()->registerStatus(this);

    connect(this, &SourceFormatterJob::finished, this, [this]() {
        emit hideProgress(this);
    });
}

QString SourceFormatterJob::statusName() const
{
    return i18n("Reformat Files");
}

void SourceFormatterJob::doWork()
{
    // TODO: consider to use ExecuteCompositeJob, with every file a separate subjob
    switch (m_workState) {
        case WorkIdle:
            m_workState = WorkFormat;
            m_fileIndex = 0;
            emit showProgress(this, 0, 0, 0);
            emit showMessage(this, i18np("Reformatting one file",
                                         "Reformatting %1 files",
                                         m_fileList.length()));

            QMetaObject::invokeMethod(this, "doWork", Qt::QueuedConnection);
            break;
        case WorkFormat:
            if (m_fileIndex < m_fileList.length()) {
                emit showProgress(this, 0, m_fileList.length(), m_fileIndex);
                formatFile(m_fileList[m_fileIndex]);

                // trigger formatting of next file
                ++m_fileIndex;
                QMetaObject::invokeMethod(this, "doWork", Qt::QueuedConnection);
            } else {
                m_workState = WorkIdle;
                emitResult();
            }
            break;
        case WorkCancelled:
            break;
    }
}

void SourceFormatterJob::start()
{
    if (m_workState != WorkIdle)
        return;

    m_workState = WorkIdle;

    QMetaObject::invokeMethod(this, "doWork", Qt::QueuedConnection);
}

bool SourceFormatterJob::doKill()
{
    m_workState = WorkCancelled;
    return true;
}

void SourceFormatterJob::setFiles(const QList<QUrl>& fileList)
{
    m_fileList = fileList;
}

void SourceFormatterJob::formatFile(const QUrl& url)
{
    qCDebug(SHELL) << "Checking whether to format file" << url;
    SourceFormatterController::FileFormatter ff(url);
    if (!ff.readFormatterAndStyle(m_sourceFormatterController->formatters())) {
        return; // unsupported MIME type or no configured formatter for it
    }

    // if the file is opened in the editor, format the text in the editor without saving it
    auto doc = ICore::self()->documentController()->documentForUrl(url);
    if (doc) {
        qCDebug(SHELL) << "Processing file " << url << "opened in editor";
        ff.formatDocument(*doc);
        return;
    }

    qCDebug(SHELL) << "Processing file " << url;
    auto getJob = KIO::storedGet(url);
    // TODO: make also async and use start() and integrate using setError and setErrorString.
    if (getJob->exec()) {
        // TODO: really fromLocal8Bit/toLocal8Bit? no encoding detection? added in b8062f736a2bf2eec098af531a7fda6ebcdc7cde
        QString text = QString::fromLocal8Bit(getJob->data());
        text = ff.format(text);
        text = ff.addModeline(text);

        auto putJob = KIO::storedPut(text.toLocal8Bit(), url, -1, KIO::Overwrite);
        // see getJob
        if (!putJob->exec()) {
            auto* message = new Sublime::Message(putJob->errorString(), Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
        }
    } else {
        auto* message = new Sublime::Message(getJob->errorString(), Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }
}
