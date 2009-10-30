
/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef CONTEXTBROWSERPLUGIN_H
#define CONTEXTBROWSERPLUGIN_H

#include <QtCore/QVariant>
#include <QSet>
#include <QMap>
#include <QList>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/rangefeedback.h>
#include <interfaces/iplugin.h>
#include <language/duchain/duchainpointer.h>
#include <language/editor/simplecursor.h>
#include <language/editor/simplerange.h>
#include <language/duchain/declaration.h>

namespace KDevelop {
  class IDocument;
  class ILanguage;
  class ParseJob;
  class DUContext;
  class TopDUContext;
  class DUChainBase;
}

namespace KTextEditor {
  class View;
}

using namespace KDevelop;

class ContextBrowserViewFactory;
class ContextBrowserView;

QWidget* masterWidget(QWidget* w);

class ContextBrowserPlugin : public KDevelop::IPlugin, public KTextEditor::SmartRangeWatcher
{
    Q_OBJECT
  public:
    ContextBrowserPlugin(QObject *parent, const QVariantList & = QVariantList() );
    virtual ~ContextBrowserPlugin();

    virtual void unload();

    void registerToolView(ContextBrowserView* view);
    void unRegisterToolView(ContextBrowserView* view);
    
    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context*);

    QWidget* toolbarWidgetForMainWindow(QWidget* widgetInWindow);
    
  public Q_SLOTS:
    void previousContextShortcut();
    void nextContextShortcut();
    
    void startDelayedBrowsing(KTextEditor::View* view);
    void stopDelayedBrowsing();
    
    void previousUseShortcut();
    void nextUseShortcut();

    void declarationSelectedInUI(DeclarationPointer decl);

    void parseJobFinished(KDevelop::ParseJob* job);
    void textDocumentCreated( KDevelop::IDocument* document );
    void documentClosed( KDevelop::IDocument* document );
    void documentDestroyed( QObject* obj );
    void viewDestroyed( QObject* obj );
    void cursorPositionChanged( KTextEditor::View* view, const KTextEditor::Cursor& newPosition );
    void viewCreated( KTextEditor::Document* , KTextEditor::View* );
    void updateViews();

    void textHintRequested(const KTextEditor::Cursor&, QString&);

    void hideTooTip();
    void findUses();
    
    void textInserted(KTextEditor::Document*, KTextEditor::Range);
    
  private:
    
    virtual void createActionsForMainWindow(Sublime::MainWindow* /*window*/, QString& xmlFile, KActionCollection& actions);
    void switchUse(bool forward);
    void clearMouseHover();
    virtual void rangeDeleted (KTextEditor::SmartRange *range);
    virtual void mouseEnteredRange(KTextEditor::SmartRange* range, KTextEditor::View* view);
    virtual void mouseExitedRange(KTextEditor::SmartRange* range, KTextEditor::View* view);

    void changeHighlight( KTextEditor::SmartRange* range, bool highlight, bool declaration, bool mouseHighlight );
    void changeHighlight( KTextEditor::View* view, KDevelop::Declaration* decl, bool highlight, bool mouseHighlight );

    void watchRange(KTextEditor::SmartRange* range);
    void ignoreRange(KTextEditor::SmartRange* range);

    void registerAsRangeWatcher(KDevelop::DUChainBase* base);
    void registerAsRangeWatcher(KDevelop::DUContext* ctx);

    /** helper for updateBrowserView().
     *  Tries to find a 'specialLanguageObject' (eg macro) in @p view under cursor @c.
     *  If found returns true and sets @p pickedLanguage to the language this object belongs to */
    bool findSpecialObject(KTextEditor::View* view, const KDevelop::SimpleCursor&, KDevelop::ILanguage*& pickedLanguage);
    KDevelop::Declaration* findDeclaration(KTextEditor::View* view, const KDevelop::SimpleCursor&, bool mouseHighlight);
    bool showDeclarationView(KTextEditor::View* view, const KDevelop::SimpleCursor&, KDevelop::Declaration* dcl, KDevelop::DUContext*);
    bool showSpecialObjectView(KTextEditor::View* view, const KDevelop::SimpleCursor&, KDevelop::ILanguage*, KDevelop::DUContext*);
    void showContextView(KTextEditor::View* view, const SimpleCursor& cursor, KDevelop::DUContext*);
    void updateBrowserWidgetFor(KTextEditor::View* view);

  private:
    //Unhighlights all currently highlighted declarations
    void unHighlightAll(KTextEditor::View* selectView = 0);
    void showToolTip(KTextEditor::View* view, KTextEditor::Cursor position);
    QTimer* m_updateTimer;
    //Must be locked before doing anything with m_backups, and _after_ the relevant smart-mutex has been locked
    QMutex m_backupsMutex;
    //Contains the range, the old attribute, and the attribute it was replaced with
    QMap<KTextEditor::SmartRange*, QPair<KTextEditor::Attribute::Ptr,KTextEditor::Attribute::Ptr> > m_backups;
    QSet<KTextEditor::View*> m_updateViews;
    QMap<KTextEditor::View*, DeclarationPointer> m_highlightedDeclarations;
    QMap<KTextEditor::View*, KTextEditor::SmartRange*> m_highlightedRange; //Special language-object range

    QSet<KTextEditor::SmartRange*> m_watchedRanges;

    //Holds a list of all active context browser tool views
    QList<ContextBrowserView*> m_views;

    //Used to override the next declaration that will be highlighted
    IndexedDeclaration m_useDeclaration;
    IndexedDeclaration m_lastHighlightedDeclaration;

    KUrl m_mouseHoverDocument;
    SimpleCursor m_mouseHoverCursor;
    ContextBrowserViewFactory* m_viewFactory;
    QPointer<QWidget> m_currentToolTip;
    IndexedDeclaration m_currentToolTipDeclaration;
    QAction* m_findUses;
    
    //This is just a marker, only used for comparison. Never expect it to be valid!
    KTextEditor::Document* m_lastInsertionDocument;
    KTextEditor::Cursor m_lastInsertionPos;
    QSet<KTextEditor::View*> m_keepHighlightedDeclaration;
    QList<QPointer<QWidget> > m_toolbarWidgets;
};

DUContext* contextAt(const SimpleCursor& position, TopDUContext* topContext);

#endif // CONTEXTBROWSERPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
