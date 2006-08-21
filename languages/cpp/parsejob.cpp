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

#include "kdevdocument.h"

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "cpplanguagesupport.h"
#include "cpphighlighting.h"

#include "parser/parsesession.h"
#include "parser/binder.h"
#include "parser/parser.h"
#include "parser/control.h"
#include "duchain/dumpchain.h"
#include "parser/rpp/preprocessor.h"
#include "duchain/cppeditorintegrator.h"
#include "duchain/dubuilder.h"
#include "duchain/ducontext.h"

CPPParseJob::CPPParseJob( const KUrl &url,
                    CppLanguageSupport *parent )
        : KDevParseJob( url, parent ),
        m_session( new ParseSession ),
        m_AST( 0 ),
        m_model( 0 ),
        m_duContext( 0 )
{
    addJob(new PreprocessJob(this));
    addJob(new ParseJob(this));
}

CPPParseJob::CPPParseJob( KDevDocument *document,
                    CppLanguageSupport *parent )
        : KDevParseJob( document, parent ),
        m_session( new ParseSession ),
        m_AST( 0 ),
        m_model( 0 ),
        m_duContext( 0 )
{
    addJob(new PreprocessJob(this));
    addJob(new ParseJob(this));
}

CPPParseJob::~CPPParseJob()
{}

KDevAST *CPPParseJob::AST() const
{
    Q_ASSERT ( isFinished () && m_AST );
    return m_AST;
}

KDevCodeModel *CPPParseJob::codeModel() const
{
    Q_ASSERT ( isFinished () && m_model );
    return m_model;
}

DUContext * CPPParseJob::duChain() const
{
    return m_duContext;
}

CppLanguageSupport * CPPParseJob::cpp() const
{
    return static_cast<CppLanguageSupport*>(const_cast<QObject*>(parent()));
}

CPPParseJob * PreprocessJob::parentJob() const
{
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

CPPParseJob * ParseJob::parentJob() const
{
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
{
}

void CPPParseJob::setAST(TranslationUnitAST * ast)
{
    m_AST = ast;
}

void CPPParseJob::setCodeModel(CodeModel * model)
{
    m_model = model;
}

void CPPParseJob::setDUChain(DUContext * duChain)
{
    m_duContext = duChain;
}

ParseJob::ParseJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
{
}

void PreprocessJob::run()
{
    bool readFromDisk = !parentJob()->openDocument();

    QString contents;

    if ( readFromDisk )
    {
        QFile file( parentJob()->document().path() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            parentJob()->setErrorMessage(i18n( "Could not open file '%1'", parentJob()->document().path() ));
            kWarning( 9007 ) << k_funcinfo << "Could not open file " << parentJob()->document() << " (path " << parentJob()->document().path() << ")" << endl;
            return ;
        }

        QByteArray fileData = file.readAll();
        contents = QString::fromUtf8( fileData.constData() );
        assert( !contents.isEmpty() );
        file.close();
    }
    else
    {
        contents = parentJob()->contentsFromEditor();
    }

    kDebug( 9007 ) << "===-- PARSING --===> "
    << parentJob()->document().fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << contents.length()
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

    parentJob()->parseSession()->setContents( preprocessor.processString( contents ).toUtf8() );
    parentJob()->parseSession()->macros = preprocessor.macros();
}

void ParseJob::run()
{
    Parser parser( new Control() );

    TranslationUnitAST* ast = parser.parse( parentJob()->parseSession() );
    ast->language = parentJob()->cpp();
    ast->session = parentJob()->parseSession();

    parentJob()->setAST(ast);

    if ( ast )
    {
        CodeModel* model = new CodeModel;
        Binder binder( model, parentJob()->parseSession() );
        binder.run( parentJob()->document(), ast );

        parentJob()->setCodeModel(model);

        KTextEditor::SmartInterface* smart = 0;
        if ( parentJob()->openDocument() && parentJob()->openDocument()->textDocument() )
            smart = dynamic_cast<KTextEditor::SmartInterface*>(parentJob()->openDocument()->textDocument());

        // Lock the smart interface, if one exists
        // Locking the interface here allows all of the highlighting to update before a redraw happens, thus no flicker
        QMutexLocker lock(smart ? smart->smartMutex() : 0);

        DUBuilder dubuilder(parentJob()->parseSession());
        DUContext* topContext = dubuilder.build(parentJob()->document(), ast);
        parentJob()->setDUChain(topContext);

        if ( parentJob()->cpp()->codeHighlighting() )
            parentJob()->cpp()->codeHighlighting()->highlightDUChain( topContext );

        // Debug output...
        /*if (topContext->smartRange()) {
            DumpChain dump;
            dump.dump(ast, parentJob()->parseSession());
            dump.dump(topContext);
        }*/
    }
    //     DumpTree dumpTree;
    //     dumpTree.dump( m_AST );
}


void CPPParseJob::addIncludedFile(const QString & filename)
{
    m_includedFiles.append(filename);
}

void CPPParseJob::requestDependancies()
{
}

ParseSession * CPPParseJob::parseSession() const
{
    return m_session;
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
