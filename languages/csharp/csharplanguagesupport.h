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

#include <iplugin.h>
#include <ilanguagesupport.h>

namespace csharp
{
    class CodeModel;
    class CodeProxy;
    class CodeDelegate;
}

class CSharpLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
Q_OBJECT
Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    CSharpLanguageSupport( QObject *parent, const QStringList& args = QStringList() );
    virtual ~CSharpLanguageSupport();

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
    csharp::CodeProxy *m_codeProxy;
    csharp::CodeDelegate *m_codeDelegate;
//     CSharpHighlighting *m_highlights;
    */
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
