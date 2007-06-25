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

class CppHighlighting;
class CppCodeCompletion;
class AST;
class TranslationUnitAST;

namespace KParts { class Part; }
namespace KDevelop { class ICodeHighlighting; class IProject; class IDocument; }
namespace CppTools { class IncludePathResolver; }
namespace Cpp { class EnvironmentManager; }

class CppLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
Q_OBJECT
Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    CppLanguageSupport( QObject* parent, const QStringList& args = QStringList() );
    virtual ~CppLanguageSupport();

    QString name() const;

    KDevelop::ICodeHighlighting *codeHighlighting() const;
    KDevelop::ILanguage *language();

    KDevelop::ParseJob *createParseJob( const KUrl &url );

    void documentLoaded( TranslationUnitAST *ast, const KUrl& document );
    void releaseAST( AST *ast );

    /// Get the full path for a file based on a search through the project's
    /// include directories
    KUrl findInclude(const KUrl &source, const QString& includeName, int includeType);

private slots:
    void documentActivated(KDevelop::IDocument*);
    void documentClosed(KDevelop::IDocument*);
    void projectOpened(KDevelop::IProject *project);
    void projectClosing(KDevelop::IProject *project);
    void documentChanged( KDevelop::IDocument* document );

private:
    CppHighlighting *m_highlights;
    CppCodeCompletion *m_cc;
    CppTools::IncludePathResolver *m_includeResolver;
    Cpp::EnvironmentManager* m_lexerCache;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
