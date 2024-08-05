/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <language/duchain/problem.h>
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
class DUContext;
class TopDUContext;
class ReferencedTopDUContext;
class DUChainBase;
class AbstractNavigationWidget;
}

namespace KTextEditor {
class Document;
class View;
}

class ContextBrowserViewFactory;
class ContextBrowserView;
class ContextBrowserPlugin;
class BrowseManager;

class ContextBrowserHintProvider
    : public KTextEditor::TextHintProvider
{
public:
    explicit ContextBrowserHintProvider(ContextBrowserPlugin* plugin);
    QString textHint(KTextEditor::View* view, const KTextEditor::Cursor& position) override;

private:
    ContextBrowserPlugin* m_plugin;
};

QWidget* masterWidget(QWidget* w);

struct ViewHighlights
{
    ViewHighlights() : keep(false)
    {
    }
    // Whether the same highlighting should be kept highlighted (usually during typing)
    bool keep;
    // The declaration that is highlighted for this view
    KDevelop::IndexedDeclaration declaration;
    // Highlighted ranges. Those may also be contained by different views.
    QList<KDevelop::PersistentMovingRange::Ptr> highlights;
};

class ContextBrowserPlugin
    : public KDevelop::IPlugin
    , public KDevelop::IContextBrowser
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IContextBrowser)

public:
    explicit ContextBrowserPlugin(QObject* parent, const KPluginMetaData& metaData,
                                  const QVariantList& = QVariantList());
    ~ContextBrowserPlugin() override;

    void unload() override;

    void registerToolView(ContextBrowserView* view);
    void unRegisterToolView(ContextBrowserView* view);

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    KXMLGUIClient* createGUIForMainWindow(Sublime::MainWindow* window) override;

    ///duchain must be locked
    ///@param force When this is true, the history-entry is added, no matter whether the context is "interesting" or not
    void updateHistory(KDevelop::DUContext* context, const KTextEditor::Cursor& cursorPosition,
                       bool force = false);

    void updateDeclarationListBox(KDevelop::DUContext* context);

    void showUses(const KDevelop::DeclarationPointer& declaration) override;

    KTextEditor::Attribute::Ptr highlightedUseAttribute() const;
    KTextEditor::Attribute::Ptr highlightedSpecialObjectAttribute() const;

public Q_SLOTS:
    void showUsesDelayed(const KDevelop::DeclarationPointer& declaration);
    void previousContextShortcut();
    void nextContextShortcut();

    void startDelayedBrowsing(KTextEditor::View* view);
    void stopDelayedBrowsing();
    void invokeAction(int index);

    void previousUseShortcut();
    void nextUseShortcut();

    void declarationSelectedInUI(const KDevelop::DeclarationPointer& decl);

    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& topContext);
    void textDocumentCreated(KDevelop::IDocument* document);
    void documentActivated(KDevelop::IDocument*);
    void viewDestroyed(QObject* obj);
    void cursorPositionChanged(KTextEditor::View* view, const KTextEditor::Cursor& newPosition);
    void viewCreated(KTextEditor::Document*, KTextEditor::View*);
    void updateViews();

    void hideToolTip();
    void findUses();

    void textInserted(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor, const QString& text);
    void selectionChanged(KTextEditor::View*);

    void historyNext();
    void historyPrevious();

    void colorSetupChanged();

private Q_SLOTS:
    // history browsing
    void documentJumpPerformed(KDevelop::IDocument* newDocument,
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
    void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile,
                                    KActionCollection& actions) override;
    QWidget* navigationWidgetForPosition(KTextEditor::View* view, KTextEditor::Cursor position,
                                         KTextEditor::Range& itemRange);
    void switchUse(bool forward);
    void clearMouseHover();

    void addHighlight(KTextEditor::View* view, KDevelop::Declaration* decl);

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
    ContextBrowserView* browserViewForWidget(QWidget* widget) const;

    void showToolTip(KTextEditor::View* view, KTextEditor::Cursor position);
    QTimer* m_updateTimer;

    //Contains the range, the old attribute, and the attribute it was replaced with
    QSet<KTextEditor::View*> m_updateViews;
    QMap<KTextEditor::View*, ViewHighlights> m_highlightedRanges;

    //Holds a list of all active context browser tool views
    QList<ContextBrowserView*> m_views;

    QVector<KTextEditor::View*> m_textHintProvidedViews;

    //Used to override the next declaration that will be highlighted
    KDevelop::IndexedDeclaration m_useDeclaration;
    KDevelop::IndexedDeclaration m_lastHighlightedDeclaration;

    QUrl m_mouseHoverDocument;
    KTextEditor::Cursor m_mouseHoverCursor;
    ContextBrowserViewFactory* m_viewFactory;
    QPointer<QWidget> m_currentToolTip;
    QPointer<QWidget> m_currentNavigationWidget;
    KDevelop::IndexedDeclaration m_currentToolTipDeclaration;
    QVector<KDevelop::IProblem::Ptr> m_currentToolTipProblems;
    QAction* m_findUses;

    QPointer<KTextEditor::Document> m_lastInsertionDocument;
    KTextEditor::Cursor m_lastInsertionPos;

    // outline toolbar
    QPointer<QLineEdit> m_outlineLine;
    QPointer<QHBoxLayout> m_toolbarWidgetLayout;
    QPointer<QWidget> m_toolbarWidget;

    // history browsing
    struct HistoryEntry
    {
        //Duchain must be locked
        explicit HistoryEntry(
            KDevelop::IndexedDUContext ctx = KDevelop::IndexedDUContext(),
            const KTextEditor::Cursor& cursorPosition = KTextEditor::Cursor());
        explicit HistoryEntry(const KDevelop::DocumentCursor& pos);
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
    QList<KDevelop::IndexedDeclaration> m_listDeclarations;
    KDevelop::IndexedString m_listUrl;
    BrowseManager* m_browseManager;
    //Used to not record jumps triggered by the context-browser as history entries
    QPointer<QWidget> m_focusBackWidget;
    int m_nextHistoryIndex;

    mutable KTextEditor::Attribute::Ptr m_highlightAttribute;

    friend class ContextBrowserHintProvider;
    ContextBrowserHintProvider m_textHintProvider;
};

#endif // KDEVPLATFORM_PLUGIN_CONTEXTBROWSERPLUGIN_H
