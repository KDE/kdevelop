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

#include <iplugin.h>
#include <ilanguagesupport.h>

class CodeModel;
class CodeProxy;
class CodeDelegate;
class CodeAggregate;
class CppHighlighting;
class CppCodeCompletion;
class TranslationUnitAST;

namespace KParts { class Part; }
namespace KDevelop { class ICodeHighlighting; class IProject; class IDocument; }

class CppLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
Q_OBJECT
Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    CppLanguageSupport( QObject* parent, const QStringList& args = QStringList() );
    virtual ~CppLanguageSupport();

    virtual QString name() const;

    virtual KDevelop::ILanguage *language();

    virtual KDevelop::ParseJob *createParseJob( const KUrl &url );

    //KDevelop::LanguageSupport implementation
/*    virtual KDevelop::CodeModel *codeModel( const KUrl& url ) const;
    virtual KDevelop::CodeProxy *codeProxy() const;
    virtual KDevelop::CodeDelegate *codeDelegate() const;
    virtual KDevelop::CodeRepository *codeRepository() const;
    virtual KDevelop::ParseJob *createParseJob( KDevelop::Document *document );
    virtual QStringList mimeTypes() const;
*/
    virtual KDevelop::ICodeHighlighting *codeHighlighting() const;
/*
    virtual void releaseAST( KDevelop::AST *ast);
*/
    virtual void documentLoaded( TranslationUnitAST *ast, const KUrl& document );

    /// Get the full path for a file based on a search through the project's
    /// include directories
    KUrl findInclude(const KUrl &source, const QString& includeName );

private slots:
/*    void documentLoaded( KDevelop::Document *document );
    void documentClosed( KDevelop::Document *document );
    void documentActivated( KDevelop::Document *document );
*/
    void documentActivated(KDevelop::IDocument* );
    void documentClosed(KDevelop::IDocument*);
    void projectOpened(KDevelop::IProject *project);
    void projectClosing(KDevelop::IProject *project);

private:
/*    QStringList m_mimetypes;
    CodeProxy *m_codeProxy;
    CodeDelegate *m_codeDelegate;
*/
    CppHighlighting *m_highlights;
    CppCodeCompletion* m_cc;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
