/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVCSHARPLANGUAGESUPPORT_H
#define KDEVCSHARPLANGUAGESUPPORT_H

#include <kdevlanguagesupport.h>

namespace csharp
{
    class CodeModel;
    class CodeProxy;
    class CodeDelegate;
}

class CSharpLanguageSupport : public Koncrete::LanguageSupport
{
Q_OBJECT
public:
    CSharpLanguageSupport( QObject *parent, const QStringList& args = QStringList() );
    virtual ~CSharpLanguageSupport();

    //Koncrete::LanguageSupport implementation
    virtual Koncrete::CodeModel *codeModel( const KUrl& url ) const;
    virtual Koncrete::CodeProxy *codeProxy() const;
    virtual Koncrete::CodeDelegate *codeDelegate() const;
    virtual Koncrete::CodeRepository *codeRepository() const;
    virtual Koncrete::ParseJob *createParseJob( const KUrl &url );
    virtual Koncrete::ParseJob *createParseJob( Koncrete::Document *document );
    virtual QStringList mimeTypes() const;

    virtual void read( Koncrete::AST *ast, std::ifstream &in );
    virtual void write( Koncrete::AST *ast, std::ofstream &out );

private slots:
    void documentLoaded( Koncrete::Document *document );
    void documentClosed( Koncrete::Document *document );
    void documentActivated( Koncrete::Document *document );
    void projectOpened();
    void projectClosed();

private:
    QStringList m_mimetypes;
    csharp::CodeProxy *m_codeProxy;
    csharp::CodeDelegate *m_codeDelegate;
//     CSharpHighlighting *m_highlights;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
