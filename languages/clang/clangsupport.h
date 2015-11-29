/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>
    Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#ifndef KDEVCLANGSUPPORT_H
#define KDEVCLANGSUPPORT_H

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>
#include <interfaces/ibuddydocumentfinder.h>

#include <QStringList>
#include <QVariantList>

class ClangIndex;
namespace KDevelop
{
class BasicRefactoring;
class IDocument;
}

namespace KTextEditor
{
class View;
}

class ClangSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport, public KDevelop::IBuddyDocumentFinder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ILanguageSupport)

public:
    explicit ClangSupport(QObject *parent, const QVariantList& args = QVariantList());
    virtual ~ClangSupport();

    /** Name Of the Language */
    QString name() const override;

    /** Parsejob used by background parser to parse given url */
    KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString &url) override;

    /** the code highlighter */
    KDevelop::ICodeHighlighting* codeHighlighting() const override;
    KDevelop::BasicRefactoring* refactoring() const override;

    void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;

    KTextEditor::Range specialLanguageObjectRange(const QUrl &url, const KTextEditor::Cursor& position) override;
    QPair<QUrl, KTextEditor::Cursor> specialLanguageObjectJumpCursor(const QUrl &url, const KTextEditor::Cursor& position) override;
    QWidget* specialLanguageObjectNavigationWidget(const QUrl &url, const KTextEditor::Cursor& position) override;

    ClangIndex* index();

    KDevelop::TopDUContext* standardContext(const QUrl &url, bool proxyContext = false) override;

    virtual KDevelop::ConfigPage* configPage(int number, QWidget *parent) override;

    virtual int configPages() const override;

    //BEGIN IBuddyDocumentFinder

    bool areBuddies(const QUrl &url1, const QUrl& url2) override;
    bool buddyOrder(const QUrl &url1, const QUrl& url2) override;
    QVector< QUrl > getPotentialBuddies(const QUrl &url) const override;

    //END IBuddyDocumentFinder

private slots:
    void documentActivated(KDevelop::IDocument* doc);
    void disableKeywordCompletion(KTextEditor::View* view);
    void enableKeywordCompletion(KTextEditor::View* view);

private:
    KDevelop::ICodeHighlighting *m_highlighting;
    KDevelop::BasicRefactoring *m_refactoring;
    QScopedPointer<ClangIndex> m_index;
};

#endif

