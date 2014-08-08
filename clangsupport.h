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

class ClangIndex;
class SimpleRefactoring;

class ClangSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport, public KDevelop::IBuddyDocumentFinder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ILanguageSupport)

public:
    explicit ClangSupport(QObject *parent, const QVariantList& args = QVariantList());
    virtual ~ClangSupport();

    /** Name Of the Language */
    virtual QString name() const;

    /** Parsejob used by background parser to parse given url */
    virtual KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString &url);

    /** the code highlighter */
    virtual KDevelop::ICodeHighlighting* codeHighlighting() const;
    virtual KDevelop::BasicRefactoring* refactoring() const override;

    virtual void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions);
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);

    virtual KDevelop::SimpleRange specialLanguageObjectRange(const KUrl& url,
                                                             const KDevelop::SimpleCursor& position) override;
    virtual QPair<KUrl, KDevelop::SimpleCursor> specialLanguageObjectJumpCursor(const KUrl& url,
                                                                                const KDevelop::SimpleCursor& position) override;
    virtual QWidget* specialLanguageObjectNavigationWidget(const KUrl& url,
                                                           const KDevelop::SimpleCursor& position) override;

    ClangIndex* index();

    //BEGIN IBuddyDocumentFinder

    virtual bool areBuddies(const KUrl& url1, const KUrl& url2);
    virtual bool buddyOrder(const KUrl& url1, const KUrl& url2);
    virtual QVector< KUrl > getPotentialBuddies(const KUrl& url) const;

    //END IBuddyDocumentFinder

private:
    KDevelop::ICodeHighlighting *const m_highlighting;
    SimpleRefactoring *m_refactoring;
    QScopedPointer<ClangIndex> m_index;
};

#endif

