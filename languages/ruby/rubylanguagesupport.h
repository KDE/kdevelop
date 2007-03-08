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

#ifndef KDEVRUBYLANGUAGESUPPORT_H
#define KDEVRUBYLANGUAGESUPPORT_H

#include <iplugin.h>
#include <ilanguagesupport.h>

namespace ruby
{
    class CodeModel;
    // class CodeProxy;
    // class CodeDelegate;
}

class RubyLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
Q_OBJECT
Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    RubyLanguageSupport( QObject *parent, const QStringList& args = QStringList() );
    virtual ~RubyLanguageSupport();

    virtual QString name() const;

    void registerExtensions();
    void unregisterExtensions();
    QStringList extensions() const;

    /*
    // KDevelop::LanguageSupport implementation
    virtual KDevelop::CodeModel *codeModel( const KUrl& url ) const;
    virtual KDevelop::CodeProxy *codeProxy() const;
    virtual KDevelop::CodeDelegate *codeDelegate() const;
    virtual KDevelop::CodeRepository *codeRepository() const;
    virtual KDevelop::ParseJob *createParseJob( const KUrl &url );
    virtual KDevelop::ParseJob *createParseJob( KDevelop::Document *document );
    virtual QStringList mimeTypes() const;

    virtual void read( KDevelop::AST *ast, std::ifstream &in );
    virtual void write( KDevelop::AST *ast, std::ofstream &out );
    */

private slots:
    /*
    void documentLoaded( KDevelop::Document *document );
    void documentClosed( KDevelop::Document *document );
    void documentActivated( KDevelop::Document *document );
    void projectOpened();
    void projectClosed();
    */

private:
    /*
    QStringList m_mimetypes;
    */
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
