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

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

#include "kdevcore.h"
#include "kdevdocument.h"
#include "kdevproject.h"
#include "kdevpersistenthash.h"

#include "Thread.h"

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "cpplanguagesupport.h"
#include "cpphighlighting.h"

#include "parser/parsesession.h"
#include "parser/binder.h"
#include "parser/parser.h"
#include "parser/control.h"
#include "duchain/dumpchain.h"
#include "duchain/cppeditorintegrator.h"
#include "duchain/declarationbuilder.h"
#include "duchain/usebuilder.h"
#include "duchain/topducontext.h"
#include "preprocessjob.h"

CPPParseJob::CPPParseJob( const KUrl &url,
                    CppLanguageSupport *parent )
        : KDevParseJob( url, parent ),
        m_session( new ParseSession ),
        m_AST( 0 ),
        m_model( 0 ),
        m_duContext( 0 ),
        m_readFromDisk( false )
{
    PreprocessJob* ppj;
    addJob(ppj = new PreprocessJob(this));
    addJob(m_parseJob = new ParseJob(this));

    // Higher priority means it will be preferred over other waiting preprocess jobs
    m_parseJob->setPriority(1);

    //kDebug() << k_funcinfo << "Created job " << this << " pp " << ppj << " parse " << parseJob() << endl;
}

CPPParseJob::CPPParseJob( KDevDocument *document,
                    CppLanguageSupport *parent )
        : KDevParseJob( document, parent ),
        m_session( new ParseSession ),
        m_AST( 0 ),
        m_model( 0 ),
        m_duContext( 0 ),
        m_readFromDisk( false )
{
    PreprocessJob* ppj;
    addJob(ppj = new PreprocessJob(this));
    addJob(m_parseJob = new ParseJob(this));

    //kDebug() << k_funcinfo << "Created job " << this << " pp " << ppj << " parse " << parseJob() << endl;
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

TopDUContext* CPPParseJob::duChain() const
{
    return m_duContext;
}

CppLanguageSupport * CPPParseJob::cpp() const
{
    return static_cast<CppLanguageSupport*>(const_cast<QObject*>(parent()));
}

CPPParseJob * ParseJob::parentJob() const
{
    Q_ASSERT(parent());
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

void CPPParseJob::setAST(TranslationUnitAST * ast)
{
    m_AST = ast;
}

void CPPParseJob::setCodeModel(CodeModel * model)
{
    m_model = model;
}

void CPPParseJob::setDUChain(TopDUContext * duChain)
{
    m_duContext = duChain;
}

ParseJob::ParseJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_priority(0)
{
}

void ParseJob::run()
{
    kDebug( 9007 ) << "===-- PARSING --===> "
    << parentJob()->document().fileName()
    << endl;

    if (parentJob()->abortRequested())
        return parentJob()->abortJob();

    QMutexLocker lock(parentJob()->cpp()->parseMutex(thread()));

    Parser parser( new Control() );

    TranslationUnitAST* ast = parser.parse( parentJob()->parseSession() );

    if (parentJob()->abortRequested())
        return parentJob()->abortJob();

    if ( ast )
    {
        ast->language = parentJob()->cpp();
        ast->session = parentJob()->parseSession();
    }

    parentJob()->setAST(ast);

    if ( ast )
    {
        CodeModel* model = new CodeModel;
        Binder binder( model, parentJob()->parseSession() );
        binder.run( parentJob()->document(), ast );

        if (parentJob()->abortRequested())
            return parentJob()->abortJob();

        parentJob()->setCodeModel(model);

        QList<DUContext*> chains;

        if (KDevCore::activeProject()) {
            foreach (const QString& include, parentJob()->includedFiles()) {
                KDevAST* ast = KDevCore::activeProject()->persistentHash()->retrieveAST(include);
                if (ast) {
                    TranslationUnitAST* t = static_cast<TranslationUnitAST*>(ast);
                    if (t->ducontext)
                        chains.append(t->ducontext);
                }
            }
        }

        TopDUContext* topContext;

        // Control the lifetime of the editor integrator (so that locking works)
        {
            CppEditorIntegrator editor(parentJob()->parseSession());

            // Translate the cursors we generate with edits that have happened since retrieval of the document source.
            if (editor.smart())
              editor.smart()->useRevision(parentJob()->revisionToken() == -1 ? 0 : parentJob()->revisionToken());

            DeclarationBuilder definitionBuilder(&editor);
            topContext = definitionBuilder.buildDeclarations(parentJob()->document(), ast, &chains);

            if (parentJob()->abortRequested())
                return parentJob()->abortJob();

            // We save some time here by not running the use compiler if the file is not loaded... (it's only needed
            // for navigation in that case)
            // FIXME make configurable
            if (!parentJob()->wasReadFromDisk()) {
                UseBuilder useBuilder(&editor);
                useBuilder.buildUses(ast);

                if (parentJob()->abortRequested())
                    return parentJob()->abortJob();
            }

            parentJob()->setDUChain(topContext);

            if ( parentJob()->cpp()->codeHighlighting() )
                parentJob()->cpp()->codeHighlighting()->highlightDUChain( topContext );

            if (editor.smart())
              editor.smart()->clearRevision();
        }

        // Debug output...

        /*if (topContext->smartRange()) {
            DumpChain dump;
            dump.dump(ast, parentJob()->parseSession());
            dump.dump(topContext);
        }*/
    }
    //     DumpTree dumpTree;
    //     dumpTree.dump( m_AST );

    //kDebug( 9007 ) << "===-- Parsing finished --===> " << parentJob()->document().fileName() << endl;
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

const QStringList & CPPParseJob::includedFiles() const
{
    return m_includedFiles;
}

void CPPParseJob::setReadFromDisk(bool readFromDisk)
{
    m_readFromDisk = readFromDisk;
}

bool CPPParseJob::wasReadFromDisk() const
{
    return m_readFromDisk;
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

ParseJob * CPPParseJob::parseJob() const
{
    return m_parseJob;
}

int ParseJob::priority() const
{
    return m_priority;
}

void ParseJob::setPriority(int priority)
{
    m_priority = priority;
}
