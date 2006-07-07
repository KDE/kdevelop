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

#include "parsejob.h"

#include <cassert>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

#include <kdevdocumentcontroller.h>

#include "parser/binder.h"
#include "parser/parser.h"
#include "parser/control.h"
#include "parser/dumptree.h"
#include "parser/rpp/preprocessor.h"

ParseJob::ParseJob( const KUrl &url,
                    pool *memoryPool,
                    QObject* parent )
        : ThreadWeaver::Job( parent ),
        m_openDocument( 0 ),
        m_document( url ),
        m_memoryPool( memoryPool ),
        m_translationUnit( 0 ),
        m_model( 0 ),
        m_highlight( 0 )
{
}

ParseJob::ParseJob( KDevDocument * document, pool * memoryPool, QObject * parent, KTextEditor::SmartRange* highlight )
        : ThreadWeaver::Job( parent ),
        m_openDocument( document ),
        m_document( document->url() ),
        m_memoryPool( memoryPool ),
        m_translationUnit( 0 ),
        m_model( 0 ),
        m_highlight( highlight )
{
}

ParseJob::~ParseJob()
{}

KUrl ParseJob::document() const
{
    return m_document;
}

TranslationUnitAST *ParseJob::translationUnit() const
{
    Q_ASSERT ( isFinished () && m_translationUnit );
    return m_translationUnit;
}

CodeModel *ParseJob::codeModel() const
{
    Q_ASSERT ( isFinished () && m_model );
    return m_model;
}

void ParseJob::run()
{
    bool readFromDisk = m_contents.isNull();
    std::size_t size;

    QString contents;

    if ( readFromDisk )
    {
        QFile file( m_document.path() );
        if ( !file.open( QIODevice::ReadOnly ) ) {
            m_errorMessage = i18n("Could not open file '%1'", m_document.path());
            kWarning(9007) << k_funcinfo << "Could not open file " << m_document << " (path " << m_document.path() << ")" << endl;
            return ;
        }

        QByteArray fileData = file.readAll();
        contents = QString::fromUtf8( fileData.constData() );
        size = fileData.size();
        assert( !contents.isEmpty() );
        file.close();
    }
    else
    {
        contents = QString::fromUtf8( m_contents.constData() );
        size = m_contents.size();
    }

    kDebug(9007) << "===-- PARSING --===> "
    << m_document.fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << size
    << endl;

    Preprocessor preprocessor;
    QStringList includes;
    //     if ( true )
    //     {
    //         includes.append ("/usr/include");
    //         includes.append ("/usr/lib/gcc/i586-suse-linux/4.0.2/include");
    //         includes.append ("/usr/include/c++/4.0.2");
    //         includes.append ("/usr/include/c++/4.0.2/i586-suse-linux");
    //     }
    includes.append ( "." );
    preprocessor.addIncludePaths( includes );

    QString ppd = preprocessor.processString( contents );
    QByteArray preprocessed = ppd.toUtf8();

    Parser parser( new Control() );
    m_translationUnit = parser.parse( preprocessed, preprocessed.length() + 1, m_memoryPool );

    if ( m_translationUnit )
    {
        m_model = new CodeModel;
        Binder binder( m_model, &parser.token_stream, &parser.lexer, m_highlight );
        binder.run( m_document, m_translationUnit );
    }
    //     DumpTree dumpTree;
    //     dumpTree.dump( m_translationUnit );

    m_memoryPool = 0;
}

bool ParseJob::wasSuccessful( ) const
{
    return m_model;
}

const QString & ParseJob::errorMessage( ) const
{
    return m_errorMessage;
}

KTextEditor::SmartRange * ParseJob::highlight( ) const
{
    return m_highlight;
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
