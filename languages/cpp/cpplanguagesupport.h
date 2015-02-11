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
#include <language/duchain/topducontext.h>
#include <interfaces/ibuddydocumentfinder.h>

#include <QDateTime>
#include <QThread>
#include <QMutex>

namespace rpp {
class pp_macro;
}

class CppHighlighting;
class CPPParseJob;
class IncludeFileDataProvider;
class SimpleRefactoring;

namespace KDevelop {
  class ICodeHighlighting;
  class CodeCompletion;
}

///A class that helps detecting what exactly makes the UI block. To use it, just place a breakpoint on UIBlockTester::lockup() and inspect the execution-position of the main thread
class UIBlockTester : public QObject {
Q_OBJECT
  class UIBlockTesterThread : public QThread {
    public:
      UIBlockTesterThread( UIBlockTester& parent );
      void run() override;
      void stop();
    private:
      UIBlockTester& m_parent;
      bool m_stop;
  };
  friend class UIBlockTesterThread;
public:

  ///@param milliseconds when the ui locks for .. milliseconds, lockup() is called
  UIBlockTester( uint milliseconds, QObject* parent );
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

class CppLanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport,
                           public KDevelop::IBuddyDocumentFinder
{
Q_OBJECT
Q_INTERFACES( KDevelop::ILanguageSupport )

public:
    explicit CppLanguageSupport( QObject* parent, const QVariantList& args = QVariantList() );
    virtual ~CppLanguageSupport();

    QString name() const override;

    KDevelop::ICodeHighlighting *codeHighlighting() const override;
    KDevelop::BasicRefactoring* refactoring() const override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;
    KDevelop::ParseJob *createParseJob( const KDevelop::IndexedString &url ) override;
    //KDevelop::AstRepresentationPtr  generateAst(const KDevelop::TopDUContext & topContext);
    KDevelop::ICreateClassHelper* createClassHelper() const override;
    
    static CppLanguageSupport* self();

    virtual QString indentationSample() const override {
      return "class C{\n class D {\n void c() {\n int m;\n }\n }\n};\n";
    }
    
    virtual void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) override;
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
  virtual KDevelop::TopDUContext *standardContext(const QUrl &url, bool proxyContext = false) override;

    /**
     * IBuddyDocumentFinder overrides.
     */
    virtual bool areBuddies(const QUrl &url1, const QUrl& url2) override;
    virtual bool buddyOrder(const QUrl &url1, const QUrl& url2) override;
    virtual QVector<QUrl> getPotentialBuddies(const QUrl &url) const override;

public slots:
    void findIncludePathsForJob(CPPParseJob* job);

private:

    //Returns the identifier and its range under the cursor as first return-value, and the tail behind it as the second
    //If the given line is an include directive, the complete line is returned starting at the directive
    QPair<QPair<QString, KTextEditor::Range>, QString> cursorIdentifier(const QUrl &url, const KTextEditor::Cursor& position) const;

    QPair<KDevelop::TopDUContextPointer, KTextEditor::Range> importedContextForPosition(const QUrl &url, const KTextEditor::Cursor& position);

    QPair<KTextEditor::Range, const rpp::pp_macro*> usedMacroForPosition(const QUrl &url, const KTextEditor::Cursor& position);

    virtual KTextEditor::Range specialLanguageObjectRange(const QUrl &url, const KTextEditor::Cursor& position) override;

    virtual QPair<QUrl, KTextEditor::Cursor> specialLanguageObjectJumpCursor(const QUrl &url, const KTextEditor::Cursor& position) override;

    virtual QWidget* specialLanguageObjectNavigationWidget(const QUrl &url, const KTextEditor::Cursor& position) override;

    static CppLanguageSupport* m_self;

    CppHighlighting *m_highlights;
    SimpleRefactoring *m_refactoring;
    KDevelop::CodeCompletion *m_cc;

    IncludeFileDataProvider* m_quickOpenDataProvider;
    
    const QStringList m_mimeTypes;
};

#endif

