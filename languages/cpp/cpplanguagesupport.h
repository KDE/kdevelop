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
#include "includeitem.h"
#include "environmentmanager.h"

class CppHighlighting;
class CppCodeCompletion;
class AST;
class TranslationUnitAST;
class IncludeFileDataProvider;

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

    /**
     * Returns a list of all files within the include-path of the given file
     * @param addPath This path is added behind each include-path, and the content of the resulting directory used.
     * */
    QList<Cpp::IncludeItem> allFilesInIncludePath(const KUrl& source, bool local, const QString& addPath) const;
    
    ///Returns the include-path. Each dir has a trailing slash. Search should be iterated forward through the list
    ///@param problems If this is nonzero, eventual problems will be added to the list
    KUrl::List findIncludePaths(const KUrl& source, QList<KDevelop::ProblemPointer>* problems) const;

    static CppLanguageSupport* self();

    const Cpp::MacroRepository::LazySet& standardMacros() const;

    Cpp::EnvironmentManager* environmentManager() const;
/**
 * There may be multiple differnt parsed versions of a document available in the du-chain.
 * This function helps choosing the right one, by creating a standard parsing-environment,
 * and searching for a TopDUContext that fits in. If this fails, a random version is chosen.
 *
 * If simplified environment-matching is enabled, and a proxy-context is found, it returns
 * that proxy-contexts target-context, so the returned context may be used for completion etc.
 * without additional checking.
 *
 * @todo Move this somewhere more general
 *
 * @warning The du-chain must be locked before calling this.
* */
  virtual KDevelop::TopDUContext *standardContext(const KUrl& url, bool allowProxyContext = false);
  
private slots:
    void documentLoaded(KDevelop::IDocument*);
    void documentClosed(KDevelop::IDocument*);
    void projectOpened(KDevelop::IProject *project);
    void projectClosing(KDevelop::IProject *project);
    void documentChanged( KDevelop::IDocument* document );
    void documentActivated( KDevelop::IDocument* document );

private:
    static CppLanguageSupport* m_self;

    CppHighlighting *m_highlights;
    CppCodeCompletion *m_cc;
    Cpp::EnvironmentManager* m_environmentManager;
    Cpp::MacroRepository::LazySet *m_standardMacros;
    QStringList *m_standardIncludePaths;
    CppTools::IncludePathResolver *m_includeResolver;
    IncludeFileDataProvider* m_quickOpenDataProvider;
};

#endif

