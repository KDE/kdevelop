
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

#ifndef KDEVPLATFORM_PLUGIN_CONTEXTBROWSERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_CONTEXTBROWSERPLUGIN_H

#include <QVariant>
#include <QSet>
#include <QMap>
#include <QList>
#include <QUrl>
#include <QPointer>

#include <KTextEditor/TextHintInterface>
#include <interfaces/iplugin.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/declaration.h>
#include <language/duchain/indexedducontext.h>
#include <language/editor/persistentmovingrange.h>
#include <language/interfaces/iquickopen.h>
#include <language/editor/documentcursor.h>

#include <language/interfaces/icontextbrowser.h>

class QHBoxLayout;
class QMenu;
class QToolButton;

namespace Sublime {
  class MainWindow;
}

namespace KDevelop {
  class IDocument;
  class ILanguage;
  class ParseJob;
  class DUContext;
  class TopDUContext;
  class DUChainBase;
  class AbstractNavigationWidget;
}

namespace KTextEditor {
  class Document;
  class View;
}

class ContextBrowserViewFactory;
class ContextBrowserView;
class BrowseManager;

QWidget* masterWidget(QWidget* w);

struct ViewHighlights
{
  ViewHighlights() : keep(false) {
  }
  // Whether the same highlighting should be kept highlighted (usually during typing)
  bool keep;
  // The declaration that is highlighted for this view
  KDevelop::IndexedDeclaration declaration;
  // Highlighted ranges. Those may also be contained by different views.
  QList<KDevelop::PersistentMovingRange::Ptr> highlights;
};

class ContextBrowserPlugin : public KDevelop::IPlugin, public KDevelop::IContextBrowser
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IContextBrowser )
  public:
    ContextBrowserPlugin(QObject *parent, const QVariantList & = QVariantList() );
    virtual ~ContextBrowserPlugin();

    virtual void unload();

    void registerToolView(ContextBrowserView* view);
    void unRegisterToolView(ContextBrowserView* view);
    
    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context*);

    virtual KXMLGUIClient* createGUIForMainWindow( Sublime::MainWindow* window );

    ///duchain must be locked
    ///@param force When this is true, the history-entry is added, no matter whether the context is "interesting" or not
    void updateHistory(KDevelop::DUContext* context, const KTextEditor::Cursor& cursorPosition,
                       bool force = false);

    void updateDeclarationListBox(KDevelop::DUContext* context);
    void setAllowBrowsing(bool allow);

    virtual void showUses(const KDevelop::DeclarationPointer& declaration);

  public Q_SLOTS:
    void showUsesDelayed(const KDevelop::DeclarationPointer& declaration);
    void previousContextShortcut();
    void nextContextShortcut();
    
    void startDelayedBrowsing(KTextEditor::View* view);
    void stopDelayedBrowsing();
    
    void previousUseShortcut();
    void nextUseShortcut();

    void declarationSelectedInUI(const KDevelop::DeclarationPointer& decl);

    void parseJobFinished(KDevelop::ParseJob* job);
    void textDocumentCreated( KDevelop::IDocument* document );
    void documentActivated( KDevelop::IDocument* );
    void viewDestroyed( QObject* obj );
    void cursorPositionChanged( KTextEditor::View* view, const KTextEditor::Cursor& newPosition );
    void viewCreated( KTextEditor::Document* , KTextEditor::View* );
    void updateViews();

    void hideToolTip();
    void findUses();
    
    void textInserted(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor, const QString& text);
    void selectionChanged(KTextEditor::View*);

    void historyNext();
    void historyPrevious();

  private slots:
    // history browsing
    void documentJumpPerformed( KDevelop::IDocument* newDocument,
                                const KTextEditor::Cursor& newCursor,
                                KDevelop::IDocument* previousDocument,
                                const KTextEditor::Cursor& previousCursor);

    void nextMenuAboutToShow();
    void previousMenuAboutToShow();
    void actionTriggered();

    void navigateLeft();
    void navigateRight();
    void navigateUp();
    void navigateDown();
    void navigateAccept();
    void navigateBack();

  private:
    QWidget* toolbarWidgetForMainWindow(Sublime::MainWindow* window);
    virtual void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile,
                                            KActionCollection& actions);
    void switchUse(bool forward);
    void clearMouseHover();

    void addHighlight( KTextEditor::View* view, KDevelop::Declaration* decl );

    /** helper for updateBrowserView().
     *  Tries to find a 'specialLanguageObject' (eg macro) in @p view under cursor @c.
     *  If found returns true and sets @p pickedLanguage to the language this object belongs to */
    KDevelop::Declaration* findDeclaration(KTextEditor::View* view, const KTextEditor::Cursor&, bool mouseHighlight);
    void updateForView(KTextEditor::View* view);

    // history browsing
    bool isPreviousEntry(KDevelop::DUContext*, const KTextEditor::Cursor& cursor) const;
    QString actionTextFor(int historyIndex) const;
    void updateButtonState();
    void openDocument(int historyIndex);
    void fillHistoryPopup(QMenu* menu, const QList<int>& historyIndices);

    enum NavigationActionType {
      Accept,
      Back,
      Down,
      Up,
      Left,
      Right
    };
    void doNavigate(NavigationActionType action);

  private:
    
    // Returns the currently active and visible context browser view that belongs
    // to the same context (mainwindow and area) as the given widget
    ContextBrowserView* browserViewForWidget(QWidget* widget);
    
    void showToolTip(KTextEditor::View* view, KTextEditor::Cursor position);
    QTimer* m_updateTimer;
    
    //Contains the range, the old attribute, and the attribute it was replaced with
    QSet<KTextEditor::View*> m_updateViews;
    QMap<KTextEditor::View*, ViewHighlights > m_highlightedRanges;

    //Holds a list of all active context browser tool views
    QList<ContextBrowserView*> m_views;

    //Used to override the next declaration that will be highlighted
    KDevelop::IndexedDeclaration m_useDeclaration;
    KDevelop::IndexedDeclaration m_lastHighlightedDeclaration;

    QUrl m_mouseHoverDocument;
    KTextEditor::Cursor m_mouseHoverCursor;
    ContextBrowserViewFactory* m_viewFactory;
    QPointer<QWidget> m_currentToolTip;
    QPointer<QWidget> m_currentNavigationWidget;
    KDevelop::IndexedDeclaration m_currentToolTipDeclaration;
    QAction* m_findUses;
    
    QPointer<KTextEditor::Document> m_lastInsertionDocument;
    KTextEditor::Cursor m_lastInsertionPos;

    // outline toolbar
    QPointer<KDevelop::IQuickOpenLine> m_outlineLine;
    QPointer<QHBoxLayout> m_toolbarWidgetLayout;
    QPointer<QWidget> m_toolbarWidget;

    // history browsing
    struct HistoryEntry {
        //Duchain must be locked
        HistoryEntry(KDevelop::IndexedDUContext ctx = KDevelop::IndexedDUContext(), const KTextEditor::Cursor& cursorPosition = KTextEditor::Cursor());
        HistoryEntry(KDevelop::DocumentCursor pos);
        //Duchain must be locked
        void setCursorPosition(const KTextEditor::Cursor& cursorPosition);

        //Duchain does not need to be locked
        KDevelop::DocumentCursor computePosition() const;

        KDevelop::IndexedDUContext context;
        KDevelop::DocumentCursor absoluteCursorPosition;
        KTextEditor::Cursor relativeCursorPosition; //Cursor position relative to the start line of the context
        QString alternativeString;
    };

    QVector<HistoryEntry> m_history;
    QPointer<QToolButton> m_previousButton;
    QPointer<QToolButton> m_nextButton;
    QPointer<QMenu> m_previousMenu, m_nextMenu;
    QPointer<QToolButton> m_browseButton;
    QList<KDevelop::IndexedDeclaration> m_listDeclarations;
    KDevelop::IndexedString m_listUrl;
    BrowseManager* m_browseManager;
    //Used to not record jumps triggered by the context-browser as history entries
    QPointer<QWidget> m_focusBackWidget;
    int m_nextHistoryIndex;

    friend class ContextBrowserHintProvider;
};

class ContextBrowserHintProvider : public KTextEditor::TextHintProvider
{
public:
  explicit ContextBrowserHintProvider(ContextBrowserPlugin* plugin);
  virtual QString textHint(KTextEditor::View* view, const KTextEditor::Cursor& position) Q_DECL_OVERRIDE;

private:
  ContextBrowserPlugin* m_plugin;
};

#endif // KDEVPLATFORM_PLUGIN_CONTEXTBROWSERPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
