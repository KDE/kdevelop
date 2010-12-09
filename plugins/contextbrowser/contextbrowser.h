
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
#include <ktexteditor/rangefeedback.h>
#include <interfaces/iplugin.h>
#include <language/duchain/duchainpointer.h>
#include <language/editor/simplecursor.h>
#include <language/duchain/declaration.h>
#include <KUrl>
#include <language/editor/persistentmovingrange.h>

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

struct ViewHighlights
{
  ViewHighlights() : keep(false) {
  }
  // Whether the same highlighting should be kept highlighted (usually during typing)
  bool keep;
  // The declaration that is highlighted for this view
  IndexedDeclaration declaration;
  // Highlighted ranges. Those may also be contained by different views.
  QList<PersistentMovingRange::Ptr> highlights;
};

class ContextBrowserPlugin : public KDevelop::IPlugin
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
    void documentActivated( KDevelop::IDocument* );
    void viewDestroyed( QObject* obj );
    void cursorPositionChanged( KTextEditor::View* view, const KTextEditor::Cursor& newPosition );
    void viewCreated( KTextEditor::Document* , KTextEditor::View* );
    void updateViews();

    void textHintRequested(const KTextEditor::Cursor&, QString&);

    void hideTooTip();
    void findUses();
    
    void textInserted(KTextEditor::Document*, KTextEditor::Range);
    void selectionChanged(KTextEditor::View*);
    
  private:
    
    virtual void createActionsForMainWindow(Sublime::MainWindow* /*window*/, QString& xmlFile, KActionCollection& actions);
    void switchUse(bool forward);
    void clearMouseHover();

    void addHighlight( KTextEditor::View* view, KDevelop::Declaration* decl );

    /** helper for updateBrowserView().
     *  Tries to find a 'specialLanguageObject' (eg macro) in @p view under cursor @c.
     *  If found returns true and sets @p pickedLanguage to the language this object belongs to */
    KDevelop::Declaration* findDeclaration(KTextEditor::View* view, const KDevelop::SimpleCursor&, bool mouseHighlight);
    void updateForView(KTextEditor::View* view);

  private:
    
    ContextBrowserView* browserViewForTextView(KTextEditor::View* view);
    
    void showToolTip(KTextEditor::View* view, KTextEditor::Cursor position);
    QTimer* m_updateTimer;
    
    //Contains the range, the old attribute, and the attribute it was replaced with
    QSet<KTextEditor::View*> m_updateViews;
    QMap<KTextEditor::View*, ViewHighlights > m_highlightedRanges;

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
    
    QPointer<KTextEditor::Document> m_lastInsertionDocument;
    KTextEditor::Cursor m_lastInsertionPos;
    QList<QPointer<QWidget> > m_toolbarWidgets;
};

#endif // CONTEXTBROWSERPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
