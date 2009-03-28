/*
 * KDevelop C++ Language Support
 *
 * Copyright 2005 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2008 David Nolden<david.nolden.kdevelop@art-master.de>
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

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>
#include "includeitem.h"
#include "environmentmanager.h"
#include <QThread>

class CppHighlighting;
class CPPParseJob;
class CppCodeCompletion;
class AST;
class TranslationUnitAST;
class IncludeFileDataProvider;

extern QStringList headerExtensions;
extern QStringList sourceExtensions;

namespace KDevelop { class ICodeHighlighting; class IProject; class IDocument; class SimpleRange; class CodeCompletion; template<class T> class DUChainPointer; typedef DUChainPointer<TopDUContext> TopDUContextPointer;}
namespace Cpp { class MacroSet; class EnvironmentManager; }
namespace CppTools { class IncludePathResolver; }
namespace Veritas { class StubContextAction; class UUTContextAction; }

///A class that helps detecting what exactly makes the UI block. To use it, just place a breakpoint on UIBlockTester::lockup() and inspect the execution-position of the main thread
class UIBlockTester : public QObject {
        Q_OBJECT
    class UIBlockTesterThread : public QThread {
    public:
      UIBlockTesterThread( UIBlockTester& parent );
      void run();
      void stop();
    private:
      UIBlockTester& m_parent;
      bool m_stop;
    };
  friend class UIBlockTesterThread;
public:

  ///@param milliseconds when the ui locks for .. milliseconds, lockup() is called
  UIBlockTester( uint milliseconds );
  ~UIBlockTester();

private slots:
  void timer();

protected:
   virtual void lockup();

 private:
     UIBlockTesterThread m_thread;
     QDateTime m_lastTime;
     QMutex m_timeMutex;
     QTimer * m_timer;
     uint m_msecs;
};

class CppLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
Q_OBJECT
Q_INTERFACES( KDevelop::ILanguageSupport )
public:
    explicit CppLanguageSupport( QObject* parent, const QVariantList& args = QVariantList() );
    virtual ~CppLanguageSupport();

    QString name() const;

    const KDevelop::ICodeHighlighting *codeHighlighting() const;
    KDevelop::ILanguage *language();
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    KDevelop::ParseJob *createParseJob( const KUrl &url );

    /// Get the full path for a file based on a search through the project's
    /// include directories
    /// @param localPath the path from which this findInclude is issued
    /// @param skipPath this path will be skipped while searching, as needed for gcc extension #include_next
    /// @return first: The found file(not a canonical path, starts with the directory it was found in)
    ///         second: The include-path the file was found in(can be used to skip that path on #include_next)
    QPair<KUrl, KUrl> findInclude(const KUrl::List& includePaths, const KUrl& localPath, const QString& includeName, int includeType, const KUrl& skipPath, bool quiet=false) const;

    /**
     * Returns a list of all files within the include-path of the given file
     * @param addPath This path is added behind each include-path, and the content of the resulting directory used.
     * @param addIncludePaths A list of include-paths that should be used for listing, additionally to the known ones
     * @param onlyAddedIncludePaths If this is true, only the include-paths given in @p addIncludePaths will be used
     * */
    QList<Cpp::IncludeItem> allFilesInIncludePath(const KUrl& source, bool local, const QString& addPath, KUrl::List addIncludePaths = KUrl::List(), bool onlyAddedIncludePaths = false ) const;

    ///Returns the include-path. Each dir has a trailing slash. Search should be iterated forward through the list
    ///@param problems If this is nonzero, eventual problems will be added to the list
    KUrl::List findIncludePaths(const KUrl& source, QList<KDevelop::ProblemPointer>* problems) const;
    
    static CppLanguageSupport* self();

    const Cpp::ReferenceCountedMacroSet& standardMacros() const;

    const KDevelop::ParsingEnvironment* standardEnvironment() const;
    
    ///Thread-safe
    bool needsUpdate(const Cpp::EnvironmentFilePointer& file, const KUrl& localPath, const KUrl::List& includePaths ) const;

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

    ///If @param fast is true, no exhaustive search is done as fallback.
  KUrl sourceOrHeaderCandidate( const KUrl &url, bool fast = false ) const;
  
  ///Returns true if the given url is a header, looking at he known file extensions
  bool isHeader(const KUrl &url) const;
  
  QStringList standardIncludePaths() const;
  
public slots:
    void findIncludePathsForJob(CPPParseJob* job);
  
private slots:
    void projectOpened(KDevelop::IProject *project);
    void projectClosing(KDevelop::IProject *project);

    ///UI:
    void switchDefinitionDeclaration();

    void newClassWizard();

private:

    //Returns the identifier and its range under the cursor as first return-value, and the tail behind it as the second
    //If the given line is an include directive, the complete line is returned starting at the directive
    QPair<QPair<QString, KDevelop::SimpleRange>, QString> cursorIdentifier(const KUrl& url, const KDevelop::SimpleCursor& position) const;

    QPair<KDevelop::TopDUContextPointer, KDevelop::SimpleRange> importedContextForPosition(const KUrl& url, const KDevelop::SimpleCursor& position);

    QPair<KDevelop::SimpleRange, const rpp::pp_macro*> usedMacroForPosition(const KUrl& url, const KDevelop::SimpleCursor& position);

    virtual KDevelop::SimpleRange specialLanguageObjectRange(const KUrl& url, const KDevelop::SimpleCursor& position);

    virtual QPair<KUrl, KDevelop::SimpleCursor> specialLanguageObjectJumpCursor(const KUrl& url, const KDevelop::SimpleCursor& position);

    virtual QWidget* specialLanguageObjectNavigationWidget(const KUrl& url, const KDevelop::SimpleCursor& position);

    static CppLanguageSupport* m_self;

    CppHighlighting *m_highlights;
    KDevelop::CodeCompletion *m_cc, *m_missingIncludeCompletion;
    Cpp::ReferenceCountedMacroSet *m_standardMacros;
    const KDevelop::ParsingEnvironment *m_standardEnvironment;
    
    QStringList *m_standardIncludePaths;
    CppTools::IncludePathResolver *m_includeResolver;
    IncludeFileDataProvider* m_quickOpenDataProvider;
    UIBlockTester* m_blockTester;

    Veritas::StubContextAction* m_stubAction;
    Veritas::UUTContextAction* m_uutAction;
};

#endif

