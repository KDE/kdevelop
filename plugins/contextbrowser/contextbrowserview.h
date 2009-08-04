/*
  * This file is part of KDevelop
 *
 * Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef CONTEXTBROWSERVIEW_H
#define CONTEXTBROWSERVIEW_H

#include <QWidget>
#include <QVector>
#include <language/duchain/topducontext.h>
#include <language/editor/simplecursor.h>
#include <language/editor/documentcursor.h>
#include "browsemanager.h"
#include <language/duchain/indexedstring.h>
#include <language/interfaces/iquickopen.h>

class ContextBrowserPlugin;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QCheckBox;
class QMenu;
class KComboBox;
class BrowseManager;

namespace KDevelop {
class IDocument;
}

class ContextBrowserView : public QWidget {
    Q_OBJECT
    public:
        ContextBrowserView( ContextBrowserPlugin*, QWidget* parent );
        ~ContextBrowserView();

        //duchain must be locked
        void setContext(KDevelop::DUContext* context);
        void setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, bool force = false);
        void setSpecialNavigationWidget(QWidget*);
        void updateMainWidget(QWidget*);

        /** Allows a single update of the view even if in locked state.
          * This is needed to browse while the locked button is checked. */
        void allowLockedUpdate();

        ///Returns the currently locked declaration, or invalid of none is locked atm.
        KDevelop::IndexedDeclaration lockedDeclaration() const;

        void setNavigationWidget(QWidget* widget);
        
        QWidget* navigationWidget() {
            return m_navigationWidget;
        }
        
        QWidget* toolbarWidget() {
            return m_toolbarWidget;
        }
        
        ///duchain must be locked
        ///@param force When this is true, the history-entry is added, no matter whether the context is "interesting" or not
        void updateHistory(KDevelop::DUContext* context, const
        KDevelop::SimpleCursor& cursorPosition, bool force = false);
        QWidget* createWidget(KDevelop::DUContext* context);
        
        //duchain must be locked
        QWidget* createWidget(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext);

        KDevelop::IndexedDeclaration declaration() const;
        
        struct HistoryEntry {
            //Duchain must be locked
            HistoryEntry(KDevelop::IndexedDUContext ctx = KDevelop::IndexedDUContext(), const KDevelop::SimpleCursor& cursorPosition = KDevelop::SimpleCursor());
            HistoryEntry(KDevelop::DocumentCursor pos);
            //Duchain must be locked
            void setCursorPosition(const KDevelop::SimpleCursor& cursorPosition);

            //Duchain does not need to be locked
            KDevelop::DocumentCursor computePosition() const;

            KDevelop::IndexedDUContext context;
            KDevelop::DocumentCursor absoluteCursorPosition;
            KDevelop::SimpleCursor relativeCursorPosition; //Cursor position relative to the start line of the context
            QString alternativeString;
        };

        void updateDeclarationListBox(KDevelop::DUContext* context);
        
        void setAllowBrowsing(bool allow) ;
        
    Q_SIGNALS:
        void startDelayedBrowsing(KTextEditor::View*);
        void stopDelayedBrowsing();
        
    public Q_SLOTS:
        void navigateLeft();
        void navigateRight();
        void navigateUp();
        void navigateDown();
        void navigateAccept();
        void navigateBack();

        void historyNext();
        void historyPrevious();
        void nextMenuAboutToShow();
        void previousMenuAboutToShow();
        void actionTriggered();
        void switchFocusToContextBrowser();
        
    private Q_SLOTS:
        void updateLockIcon(bool); 
        void declarationMenu();
        void documentJumpPerformed( KDevelop::IDocument* newDocument, KTextEditor::Cursor newCursor, KDevelop::IDocument* previousDocument, KTextEditor::Cursor previousCursor);
        

    private:
        virtual void showEvent(QShowEvent* event);
        virtual bool event(QEvent* event);
        
        virtual void focusInEvent(QFocusEvent* event);
        virtual void focusOutEvent(QFocusEvent* event);
        bool isLocked() const;
        void resetWidget();
        
        bool isPreviousEntry(KDevelop::DUContext*, const KDevelop::SimpleCursor& cursor);
        QString actionTextFor(int historyIndex);
        void updateButtonState();
        void openDocument(int historyIndex);
        void fillHistoryPopup(QMenu* menu, const QList<int>& historyIndices);

    private:
        
        KDevelop::IndexedDeclaration m_declaration;
        
        ContextBrowserPlugin* m_plugin;
        QVBoxLayout* m_layout;
        QToolButton* m_lockButton;
        QToolButton* m_declarationMenuButton;
        
        QHBoxLayout* m_buttons;
        QPointer<QWidget> m_navigationWidget;
        KDevelop::DeclarationId m_navigationWidgetDeclaration;
        bool m_allowLockedUpdate;
        KDevelop::IndexedTopDUContext m_lastUsedTopContext;

        KDevelop::IndexedDUContext m_context;
        int m_nextHistoryIndex;
        
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
        QPointer<KDevelop::IQuickOpenLine> m_outlineLine;
        
        QPointer<QHBoxLayout> m_toolbarWidgetLayout;
        QPointer<QWidget> m_toolbarWidget;
};

#endif
