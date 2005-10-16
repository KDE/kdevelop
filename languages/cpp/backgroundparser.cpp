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

#include "backgroundparser.h"

#include <QList>
#include <QTimer>

#include <kdebug.h>

#include <ktexteditor/document.h>

#include "kdevdocumentcontroller.h"

#include <ThreadWeaver.h>
#include "preprocessjob.h"
#include "parsejob.h"

#include "parser/preprocessor.h"
#include "parser/control.h"
#include "parser/parser.h"

#include "cpplanguagesupport.h"

BackgroundParser::BackgroundParser( QObject* parent )
    : QObject( parent )
{
    m_weaver = ThreadWeaver::Weaver::instance();
    m_preprocessor = new Preprocessor( this );
    m_control = new Control();
    m_parser = new Parser( m_control );

    m_timer = new QTimer( this );
    m_timer->setSingleShot(true);
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
    {
        m_documents.insert( url, true );

        if (KTextEditor::Document* doc = static_cast<CppLanguageSupport*>(parent())->documentController()->documentForURL(url))
            connect(doc, SIGNAL(textChanged(KTextEditor::Document*)), SLOT(documentChanged(KTextEditor::Document*)));

        parseDocuments();
    }
}

void BackgroundParser::removeDocument( const KURL &url )
{
    m_documents.remove( url );
}

void BackgroundParser::parseDocuments()
{
    QList< ThreadWeaver::Job* > jobs;
    for ( QMap<KURL,bool>::Iterator it = m_documents.begin(); it != m_documents.end(); ++it )
    {
//         PreprocessJob *preprocess =
//             new PreprocessJob( url, m_preprocessor, this );
        if (it.value()) {
            ParseJob *parse = new ParseJob( it.key(), m_parser, this );
            it.value() = false;

//         parse->addDependency( preprocess );
//         jobs.append( preprocess );

            jobs.append( parse );
        }
    }
    m_weaver->enqueue( jobs );
}

void BackgroundParser::documentChanged( KTextEditor::Document * document )
{
    Q_ASSERT(m_documents.contains(document->url()));
    m_documents.insert(document->url(), true);
    if (!m_timer->isActive())
        m_timer->start( 500 );
}

#include "backgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
