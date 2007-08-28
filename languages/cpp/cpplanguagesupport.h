/*
 * KDevelop C++ Language Support
 *
 * Copyright 2005 Matt Rogers <mattr@kde.org>
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
namespace Cpp { class MacroSet; class EnvironmentManager; }
namespace CppTools { class IncludePathResolver; }

class CppLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
Q_OBJECT
Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    explicit CppLanguageSupport( QObject* parent, const QVariantList& args = QVariantList() );
    virtual ~CppLanguageSupport();

    QString name() const;

    KDevelop::ICodeHighlighting *codeHighlighting() const;
    KDevelop::ILanguage *language();

    KDevelop::ParseJob *createParseJob( const KUrl &url );

    /// Get the full path for a file based on a search through the project's
    /// include directories
    /// @param localPath the path from which this findInclude is issued
    /// @param skipPath this path will be skipped while searching, as needed for gcc extension #include_next
    /// @return first: The found file, second: The include-path the file was found in(can be used to skip that path on #include_next)
    QPair<KUrl, KUrl> findInclude(const KUrl::List& includePaths, const KUrl& localPath, const QString& includeName, int includeType, const KUrl& skipPath) const;

    ///Returns the include-path. Each dir has a trailing slash. Search should be iterated forward through the list
    KUrl::List findIncludePaths(const KUrl& source) const;

    static CppLanguageSupport* self();

    const Cpp::MacroSet& standardMacros() const;

    Cpp::EnvironmentManager* environmentManager() const;

private slots:
    void documentLoaded(KDevelop::IDocument*);
    void projectOpened(KDevelop::IProject *project);
    void projectClosing(KDevelop::IProject *project);
    void documentChanged( KDevelop::IDocument* document );

private:
    static CppLanguageSupport* m_self;

    CppHighlighting *m_highlights;
    CppCodeCompletion *m_cc;
    Cpp::EnvironmentManager* m_environmentManager;
    Cpp::MacroSet *m_standardMacros;
    QStringList *m_standardIncludePaths;
    CppTools::IncludePathResolver *m_includeResolver;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
