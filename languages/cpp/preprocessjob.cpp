/*
* KDevelop C++ Preprocess Job
*
* Copyright (c) 2005 Adam Treat <treat@kde.org>
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

#include <QFile>
#include <QTextStream>

#include <kdebug.h>

#include "parser/preprocessor.h"
#include "preprocessjob.h"

PreprocessJob::PreprocessJob( const KURL &url, Preprocessor *preprocessor,
                              QObject* parent )
        : ThreadWeaver::Job( parent ),
        m_document( url ),
        m_preprocessor( preprocessor )
{}

PreprocessJob::~PreprocessJob()
{}

void PreprocessJob::run()
{
    kdDebug() << "=====----- PREPROCESSING ----=======> "
    << m_document.fileName() << endl;

    QByteArray source;
    QFile file( m_document.fileName() );
    if ( file.open( IO_ReadOnly ) )
    {
        QTextStream text( &file );
        source = text.readAll().toLatin1();
        file.close();
    }
    m_preprocessor->run( source );
    m_preprocessor = 0;
}

#include "preprocessjob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
