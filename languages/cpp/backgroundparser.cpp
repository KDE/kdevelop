/*
 * KDevelop C++ Background Parser
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

#include <kdebug.h>

#include <QList>
#include <QTimer>

#include <ThreadWeaver.h>
#include "preprocessjob.h"
#include "parsejob.h"

#include "parser/preprocessor.h"
#include "parser/control.h"
#include "parser/parser.h"

#include "backgroundparser.h"

BackgroundParser::BackgroundParser( QObject* parent )
    : QObject( parent )
{
    m_weaver = ThreadWeaver::Weaver::instance();
    m_timer = new QTimer( this );
    m_preprocessor = new Preprocessor( this );
    m_control = new Control();
    m_parser = new Parser( m_control );

    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );
    m_timer->start( 500 );
}

BackgroundParser::~BackgroundParser()
{
    delete m_control;
    delete m_parser;
}

void BackgroundParser::addDocument( const KURL &url )
{
    if ( !m_documents.contains( url ) )
        m_documents.append( url );
}

void BackgroundParser::removeDocument( const KURL &url )
{
    m_documents.removeAll( url );
}

void BackgroundParser::parseDocuments()
{
    /*QList< ThreadWeaver::Job* > jobs;
    foreach ( KURL url, m_documents )
    {
        PreprocessJob *preprocess =
            new PreprocessJob( url, m_preprocessor, this );
        ParseJob *parse = new ParseJob( url, m_parser, this );
        parse->addDependency( preprocess );
        jobs.append( preprocess );
        jobs.append( parse );
    }
    m_weaver->enqueue( jobs );*/
}

#include "backgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
