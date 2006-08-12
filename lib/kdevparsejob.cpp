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

#include <kdevdocumentcontroller.h>

KDevParseJob::KDevParseJob( const KUrl &url,
                            QObject *parent )
        : ThreadWeaver::Job( parent ),
        m_document( url )
{}

KDevParseJob::KDevParseJob( KDevDocument *document,
                            QObject *parent )
        : ThreadWeaver::Job( parent ),
        m_document( document->url() )
{}

KDevParseJob::~KDevParseJob()
{}

KUrl KDevParseJob::document() const
{
    return m_document;
}

bool KDevParseJob::wasSuccessful() const
{
    return codeModel();
}

const QString & KDevParseJob::errorMessage() const
{
    return m_errorMessage;
}

#include "kdevparsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
