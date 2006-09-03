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

#include <kdevlanguagesupport.h>

namespace ruby
{
    class CodeModel;
}

class RubyLanguageSupport: public KDevLanguageSupport {
    Q_OBJECT
public:
    RubyLanguageSupport(QObject *parent, const QStringList &args = QStringList());
    virtual ~RubyLanguageSupport();

    //KDevLanguageSupport implementation
    virtual KDevCodeModel *codeModel( const KUrl& url ) const;
    virtual KDevCodeProxy *codeProxy() const;
    virtual KDevCodeDelegate *codeDelegate() const;
    virtual KDevCodeRepository *codeRepository() const;
    virtual KDevParseJob *createParseJob( const KUrl &url );
    virtual KDevParseJob *createParseJob( KDevDocument *document );
    virtual QStringList mimeTypes() const;

    virtual void read( KDevAST *ast, std::ifstream &in );
    virtual void write( KDevAST *ast, std::ofstream &out );

private slots:
    void documentLoaded( KDevDocument *document );
    void documentClosed( KDevDocument *document );
    void documentActivated( KDevDocument *document );
    void projectOpened();
    void projectClosed();

private:
    QStringList m_mimetypes;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
