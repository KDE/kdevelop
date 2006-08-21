/*
* This file is part of KDevelop
*
* Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#include "kdevparsejob.h"

#include <cassert>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

#include <ktexteditor/document.h>

#include <kdevdocumentcontroller.h>

KDevParseJob::KDevParseJob( const KUrl &url,
                            QObject *parent )
        : ThreadWeaver::JobSequence( parent ),
        m_document( url ),
        m_openDocument( 0 )
{}

KDevParseJob::KDevParseJob( KDevDocument *document,
                            QObject *parent )
        : ThreadWeaver::JobSequence( parent ),
        m_document( document->url() ),
        m_openDocument( document )
{}

KDevParseJob::~KDevParseJob()
{}

KUrl KDevParseJob::document() const
{
    return m_document;
}

KDevDocument* KDevParseJob::openDocument() const
{
    return m_openDocument;
}

bool KDevParseJob::wasSuccessful() const
{
    return codeModel();
}

const QString & KDevParseJob::errorMessage() const
{
    return m_errorMessage;
}

DUContext * KDevParseJob::duChain() const
{
    // No definition-use chain available by default
    return 0L;
}

QString KDevParseJob::contentsFromEditor() const
{
    return m_openDocument->textDocument()->text();
}

void KDevParseJob::setErrorMessage(const QString& message)
{
    m_errorMessage = message;
}

int KDevParseJob::priority() const
{
    if (m_openDocument)
        if (m_openDocument->isActive())
            return 2;
        else
            return 1;
    else
        return 0;
}

#include "kdevparsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
