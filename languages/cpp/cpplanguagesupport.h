/*
 * KDevelop C++ Language Support
 *
 * Copyright (c) 2005 Matt Rogers <mattr@kde.org>
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

#ifndef KDEVCPPLANGUAGESUPPORT_H
#define KDEVCPPLANGUAGESUPPORT_H

#include <kdevlanguagesupport.h>

class CodeModel;
class CodeProxy;
class CodeDelegate;
class CodeAggregate;
class CppHighlighting;
class CppCodeCompletion;

class CppLanguageSupport : public Koncrete::LanguageSupport
{
Q_OBJECT
public:
    CppLanguageSupport( QObject* parent, const QStringList& args = QStringList() );
    virtual ~CppLanguageSupport();

    //Koncrete::LanguageSupport implementation
    virtual Koncrete::CodeModel *codeModel( const KUrl& url ) const;
    virtual Koncrete::CodeProxy *codeProxy() const;
    virtual Koncrete::CodeDelegate *codeDelegate() const;
    virtual Koncrete::CodeRepository *codeRepository() const;
    virtual Koncrete::ParseJob *createParseJob( const KUrl &url );
    virtual Koncrete::ParseJob *createParseJob( Koncrete::Document *document );
    virtual QStringList mimeTypes() const;

    virtual Koncrete::CodeHighlighting *codeHighlighting() const;

    virtual void releaseAST( Koncrete::AST *ast);

    virtual void documentLoaded( Koncrete::AST *ast, const KUrl& document );

    /// Get the full path for a file based on a search through the project's
    /// include directories
    KUrl findInclude( const QString& fileName );

private slots:
    void documentLoaded( Koncrete::Document *document );
    void documentClosed( Koncrete::Document *document );
    void documentActivated( Koncrete::Document *document );
    void projectOpened();
    void projectClosing();

private:
    QStringList m_mimetypes;
    CodeProxy *m_codeProxy;
    CodeDelegate *m_codeDelegate;
    CppHighlighting *m_highlights;
    CppCodeCompletion* m_cc;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
