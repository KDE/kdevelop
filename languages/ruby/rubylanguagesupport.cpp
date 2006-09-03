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

#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>

#include <kdevast.h>
#include "parser/ruby_parser.h"
#include "parser/ruby_serialize_visitor.h"

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevfilemanager.h>
#include <kdevprojectmodel.h>
#include <kdevprojectcontroller.h>
#include <kdevdocumentcontroller.h>
#include <kdevbackgroundparser.h>
#include "parsejob.h"

#include "rubylanguagesupport.h"

#include <kdebug.h>

using namespace ruby;

typedef KGenericFactory<RubyLanguageSupport> KDevRubySupportFactory;
K_EXPORT_COMPONENT_FACTORY(kdevrubylanguagesupport,
                           KDevRubySupportFactory( "kdevrubysupport" ))

RubyLanguageSupport::RubyLanguageSupport(QObject *parent, const QStringList& /*args*/)
    :KDevLanguageSupport(KDevRubySupportFactory::instance(), parent)
{
    QString types = QLatin1String("application/x-ruby");
    m_mimetypes = types.split(",");

    connect(KDevCore::documentController(), SIGNAL(documentLoaded(KDevDocument*)),
        this, SLOT(documentLoaded(KDevDocument*)));
    connect(KDevCore::documentController(), SIGNAL(documentClosed(KDevDocument*)),
        this, SLOT(documentClosed(KDevDocument*)));
    connect(KDevCore::documentController(), SIGNAL(documentActivated(KDevDocument*)),
        this, SLOT(documentActivated(KDevDocument*)));
    connect(KDevCore::projectController(), SIGNAL(projectOpened()),
        this, SLOT(projectOpened()));
    connect(KDevCore::projectController(), SIGNAL(projectClosed()),
        this, SLOT(projectClosed()));
}

RubyLanguageSupport::~RubyLanguageSupport()
{
}

KDevCodeModel *RubyLanguageSupport::codeModel(const KUrl &url) const
{
    return 0;
}

KDevCodeProxy *RubyLanguageSupport::codeProxy() const
{
    return 0;
}

KDevCodeDelegate *RubyLanguageSupport::codeDelegate() const
{
    return 0;
}

KDevCodeRepository *RubyLanguageSupport::codeRepository() const
{
    return 0;
}

KDevParseJob *RubyLanguageSupport::createParseJob(const KUrl &url)
{
    return new ParseJob(url, this);
}

KDevParseJob *RubyLanguageSupport::createParseJob(KDevDocument *document)
{
    return new ParseJob(document, this);
}

QStringList RubyLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void RubyLanguageSupport::read(KDevAST *ast, std::ifstream &in)
{
    //FIXME Need to attach the memory pool to the ast somehow so it is saved
    parser::memory_pool_type memory_pool;

    // This is how we read the AST from a file
    if (in.is_open())
    {
        serialize::read(&memory_pool, static_cast<ast_node*>(ast), &in);
    }
}

void RubyLanguageSupport::write(KDevAST *ast, std::ofstream &out)
{
    // This is how we save the AST to a file
    if (out.is_open())
    {
        serialize::write(static_cast<ast_node*>(ast), &out);
    }
}

void RubyLanguageSupport::documentLoaded(KDevDocument *document)
{
    if (supportsDocument(document))
        KDevCore::backgroundParser()->addDocument(document);
}

void RubyLanguageSupport::documentClosed(KDevDocument *document)
{
    if (supportsDocument(document))
        KDevCore::backgroundParser()->removeDocument(document);
}

void RubyLanguageSupport::documentActivated(KDevDocument *document)
{
    Q_UNUSED(document);
}

void RubyLanguageSupport::projectOpened()
{
    KUrl::List documentList;
    QList<KDevProjectFileItem*> files = KDevCore::activeProject()->allFiles();
    foreach (KDevProjectFileItem *file, files)
    {
        if (file->url().fileName().endsWith( ".rb" ))
        {
            documentList.append(file->url());
        }
    }
    KDevCore::backgroundParser()->addDocumentList(documentList);
}

void RubyLanguageSupport::projectClosed()
{
    // FIXME This should remove the project files from the backgroundparser
}

#include "rubylanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
