/*****************************************************************************
 * Copyright (c) 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/

#include "parsejob.h"

#include <cassert>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <kdevcodemodel.h>

#include "Thread.h"

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

#include "parser/ruby_parser.h"

#include "rubylanguagesupport.h"

using namespace ruby;

ParseJob::ParseJob(const KUrl &url, RubyLanguageSupport *parent)
    :KDevParseJob(url, parent), m_AST(0), m_model(0)
{
}

ParseJob::ParseJob(KDevDocument *document, RubyLanguageSupport *parent)
    :KDevParseJob(document, parent), m_AST(0), m_model(0)
{
}

ParseJob::~ParseJob()
{
}

RubyLanguageSupport *ParseJob::ruby() const
{
    return static_cast<RubyLanguageSupport*>(const_cast<QObject*>(parent()));
}

KDevAST *ParseJob::AST() const
{
    Q_ASSERT(isFinished() && m_AST);
    return m_AST;
}

KDevCodeModel *ParseJob::codeModel() const
{
    Q_ASSERT(isFinished() && m_model);
    return m_model;
}

void ParseJob::run()
{
    if (abortRequested())
        return abortJob();

    QMutexLocker lock(ruby()->parseMutex(thread()));

    bool readFromDisk = !contentsAvailableFromEditor();

    char *contents;
    QByteArray fileData;

    if (readFromDisk)
    {
        QFile file(m_document.path());
        if (!file.open(QIODevice::ReadOnly))
        {
            m_errorMessage = i18n("Could not open file '%1'", m_document.path());
            kWarning( 9007 ) << k_funcinfo << "Could not open file " << m_document
                             << " (path " << m_document.path() << ")" << endl;
            return ;
        }

        fileData = file.readAll();
        QString qcontents = QString::fromUtf8( fileData.constData() );
        contents = fileData.data();
        Q_ASSERT (!qcontents.isEmpty());
        file.close();
    }
    else
    {
        fileData = contentsFromEditor().toAscii();
        contents = fileData.data();
    }

    kDebug() << "===-- PARSING --===> "
    << m_document.fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << fileData.length()
    << endl;

    if (abortRequested())
        return abortJob();

    parser::token_stream_type token_stream;
    parser::memory_pool_type memory_pool;

    // 0) setup
    parser ruby_parser;
    ruby_parser.set_token_stream(&token_stream);
    ruby_parser.set_memory_pool(&memory_pool);

    // 1) tokenize
    ruby_parser.tokenize(contents);

    if (abortRequested())
        return abortJob();

    // 2) parse
    bool matched = ruby_parser.parse_program(&m_AST);
    m_model = new CodeModel;

    if (abortRequested())
        return abortJob();

    if (matched)
    {
        //TODO: bind declarations to the code model
    }
    else
    {
        ruby_parser.yy_expected_symbol(ast_node::Kind_program, "program");
    }
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
