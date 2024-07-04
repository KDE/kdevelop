/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVCLANGSUPPORT_H
#define KDEVCLANGSUPPORT_H

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>
#include <interfaces/ibuddydocumentfinder.h>

#include <QVariantList>

class ClangIndex;
class ClangRefactoring;
namespace KDevelop
{
class IDocument;
}

namespace KTextEditor
{
class View;
class Document;
}

class ClangSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport, public KDevelop::IBuddyDocumentFinder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ILanguageSupport)

public:
    explicit ClangSupport(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args = QVariantList());
    ~ClangSupport() override;

    /** Name Of the Language */
    QString name() const override;

    /** Parsejob used by background parser to parse given url */
    KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString &url) override;

    /** the code highlighter */
    KDevelop::ICodeHighlighting* codeHighlighting() const override;
    KDevelop::BasicRefactoring* refactoring() const override;
    KDevelop::ICreateClassHelper* createClassHelper() const override;

    void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    KTextEditor::Range specialLanguageObjectRange(const QUrl &url, const KTextEditor::Cursor& position) override;
    QPair<QUrl, KTextEditor::Cursor> specialLanguageObjectJumpCursor(const QUrl &url, const KTextEditor::Cursor& position) override;
    QPair<QWidget*, KTextEditor::Range> specialLanguageObjectNavigationWidget(const QUrl& url,
                                                                              const KTextEditor::Cursor& position) override;

    QString indentationSample() const override;

    ClangIndex* index();

    KDevelop::TopDUContext* standardContext(const QUrl &url, bool proxyContext = false) override;

    KDevelop::ConfigPage* configPage(int number, QWidget *parent) override;

    int configPages() const override;

    int suggestedReparseDelayForChange(KTextEditor::Document* doc, const KTextEditor::Range& changedRange,
                                       const QString& changedText, bool removal) const override;

    //BEGIN IBuddyDocumentFinder

    bool areBuddies(const QUrl &url1, const QUrl& url2) override;
    bool buddyOrder(const QUrl &url1, const QUrl& url2) override;
    QVector<QUrl> potentialBuddies(const QUrl& url) const override;

    //END IBuddyDocumentFinder

private Q_SLOTS:
    void documentActivated(KDevelop::IDocument* doc);
    void disableKeywordCompletion(KTextEditor::View* view);
    void enableKeywordCompletion(KTextEditor::View* view);

private:
    KDevelop::ICodeHighlighting *m_highlighting;
    ClangRefactoring *m_refactoring;
    QScopedPointer<ClangIndex> m_index;
};

#endif

