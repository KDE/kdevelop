/*
 * This file is part of KDevelop
 *
 * Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
 * Copyright 2009 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "sourceformatterjob.h"

#include "sourceformattercontroller.h"

#include <debug.h>

#include <QMimeDatabase>
#include <QTextStream>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/isourceformatter.h>
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
    // check mimetype
    QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
    qCDebug(SHELL) << "Checking file " << url << " of mime type " << mime.name();
    auto formatter = m_sourceFormatterController->formatterForUrl(url, mime);
    if (!formatter) // unsupported mime type
        return;

    // if the file is opened in the editor, format the text in the editor without saving it
    auto doc = ICore::self()->documentController()->documentForUrl(url);
    if (doc) {
        qCDebug(SHELL) << "Processing file " << url << "opened in editor";
        m_sourceFormatterController->formatDocument(doc, formatter, mime);
        return;
    }

    qCDebug(SHELL) << "Processing file " << url;
    auto getJob = KIO::storedGet(url);
    // TODO: make also async and use start() and integrate using setError and setErrorString.
    if (getJob->exec()) {
        // TODO: really fromLocal8Bit/toLocal8Bit? no encoding detection? added in b8062f736a2bf2eec098af531a7fda6ebcdc7cde
        QString text = QString::fromLocal8Bit(getJob->data());
        text = formatter->formatSource(text, url, mime);
        text = m_sourceFormatterController->addModelineForCurrentLang(text, url, mime);

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
