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

#include "rubylanguagesupport.h"

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>

#include <QExtensionFactory>

/*
#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevfilemanager.h>
#include <kdevprojectmodel.h>
#include <kdevprojectcontroller.h>
#include <kdevdocumentcontroller.h>
#include <kdevbackgroundparser.h>

#include <kdevast.h>
*/

// from the parser subdirectory
#include <ruby_parser.h>
#include <ruby_serialize_visitor.h>

#include "parsejob.h"

using namespace ruby;

typedef KGenericFactory<RubyLanguageSupport> KDevRubySupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevrubylanguagesupport, KDevRubySupportFactory( "kdevrubysupport" ) )

KDEV_USE_EXTENSION_INTERFACE_NS( KDevelop, ILanguageSupport, RubyLanguageSupport )

RubyLanguageSupport::RubyLanguageSupport( QObject* parent,
                                          const QStringList& /*args*/ )
        : KDevelop::IPlugin( KDevRubySupportFactory::componentData(), parent )
        , KDevelop::ILanguageSupport()
{
    /*
    QString types = QLatin1String("application/x-ruby");
    m_mimetypes = types.split(",");

    connect(KDevelop::Core::documentController(), SIGNAL(documentLoaded(KDevelop::Document*)),
        this, SLOT(documentLoaded(KDevelop::Document*)));
    connect(KDevelop::Core::documentController(), SIGNAL(documentClosed(KDevelop::Document*)),
        this, SLOT(documentClosed(KDevelop::Document*)));
    connect(KDevelop::Core::documentController(), SIGNAL(documentActivated(KDevelop::Document*)),
        this, SLOT(documentActivated(KDevelop::Document*)));
    connect(KDevelop::Core::projectController(), SIGNAL(projectOpened()),
        this, SLOT(projectOpened()));
    connect(KDevelop::Core::projectController(), SIGNAL(projectClosed()),
        this, SLOT(projectClosed()));
    */
}

RubyLanguageSupport::~RubyLanguageSupport()
{
}

/*
KDevelop::CodeModel *RubyLanguageSupport::codeModel(const KUrl &url) const
{
    Q_UNUSED( url );
    return 0;
}

KDevelop::CodeProxy *RubyLanguageSupport::codeProxy() const
{
    return 0;
}

KDevelop::CodeDelegate *RubyLanguageSupport::codeDelegate() const
{
    return 0;
}

KDevelop::CodeRepository *RubyLanguageSupport::codeRepository() const
{
    return 0;
}

KDevelop::ParseJob *RubyLanguageSupport::createParseJob(const KUrl &url)
{
    return new ParseJob(url, this);
}

KDevelop::ParseJob *RubyLanguageSupport::createParseJob(KDevelop::Document *document)
{
    return new ParseJob(document, this);
}

QStringList RubyLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void RubyLanguageSupport::read(KDevelop::AST *ast, std::ifstream &in)
{
    //FIXME Need to attach the memory pool to the ast somehow so it is saved
    parser::memory_pool_type memory_pool;

    // This is how we read the AST from a file
    if (in.is_open())
    {
        serialize::read(&memory_pool, static_cast<ast_node*>(ast), &in);
    }
}

void RubyLanguageSupport::write(KDevelop::AST *ast, std::ofstream &out)
{
    // This is how we save the AST to a file
    if (out.is_open())
    {
        serialize::write(static_cast<ast_node*>(ast), &out);
    }
}

void RubyLanguageSupport::documentLoaded(KDevelop::Document *document)
{
    if (supportsDocument(document))
        KDevelop::Core::backgroundParser()->addDocument(document);
}

void RubyLanguageSupport::documentClosed(KDevelop::Document *document)
{
    if (supportsDocument(document))
        KDevelop::Core::backgroundParser()->removeDocument(document);
}

void RubyLanguageSupport::documentActivated(KDevelop::Document *document)
{
    Q_UNUSED(document);
}

void RubyLanguageSupport::projectOpened()
{
    KUrl::List documentList;
    QList<KDevelop::ProjectFileItem*> files = KDevelop::Core::activeProject()->allFiles();
    foreach (KDevelop::ProjectFileItem *file, files)
    {
        if (file->url().fileName().endsWith( ".rb" ))
        {
            documentList.append(file->url());
        }
    }
    KDevelop::Core::backgroundParser()->addDocumentList(documentList);
}

void RubyLanguageSupport::projectClosed()
{
    // FIXME This should remove the project files from the backgroundparser
}
*/

QString RubyLanguageSupport::name() const
{
    return "Ruby";
}

QStringList RubyLanguageSupport::extensions() const
{
    return QStringList() << "ILanguageSupport";
}

#include "rubylanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
